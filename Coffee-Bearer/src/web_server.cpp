#include "web_server.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <NTPClient.h>
#include "settings.h"
#include "credentials.h"
#include "user_management.h"
#include "coffee_machine.h"
#include "persistence.h"

// --- NOVO SISTEMA DE SESSÃO SEGURO ---
String session_token = "";
unsigned long session_start_time = 0;
#define SESSION_TIMEOUT_MS 600000 // 10 minutos de timeout (600,000 ms)

// Função para gerar um token aleatório e seguro
String generate_token() {
    String token = "";
    for (int i = 0; i < 24; i++) {
        token += String((char)random(65, 122)); // Gera caracteres aleatórios (A-Z, a-z)
    }
    return token;
}
// ------------------------------------

extern AsyncWebServer server;
extern DNSServer dnsServer;
extern Preferences preferences;
extern NTPClient timeClient;
extern Usuario usuarios[MAX_USUARIOS];
extern int total_usuarios;
extern int total_cafes_servidos;
extern bool sistema_ocupado;
extern String ultimo_evento;
extern int cafes_restantes;

extern void registrar_log(String evento);

// As funções startProvisioningMode() e connectToWiFi() permanecem as mesmas
void startProvisioningMode() {
    const char* ap_ssid = "Configurar-Cafeteira";
    Serial.println("\nNenhuma credencial Wi-Fi encontrada.");
    Serial.print("Iniciando Modo de Configuração. Conecte-se a rede: ");
    Serial.println(ap_ssid);
    WiFi.softAP(ap_ssid);
    dnsServer.start(53, "*", WiFi.softAPIP());
    server.on("/setup", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/setup.html", "text/html");
        });
    server.on("/save_credentials", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();
            preferences.begin("cafeteira", false);
            preferences.putString("wifi_ssid", ssid);
            preferences.putString("wifi_pass", password);
            preferences.end();
            request->send(200, "text/plain", "Credenciais salvas com sucesso! O dispositivo será reiniciado.");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "SSID ou senha ausentes.");
        }
        });
    server.onNotFound([](AsyncWebServerRequest* request) {
        request->redirect("/setup");
        });
    server.begin();
    while (true) {
        dnsServer.processNextRequest();
        delay(10);
    }
}

bool connectToWiFi(String ssid, String password) {
    Serial.print("Conectando a ");
    Serial.println(ssid);
    if (!WiFi.config(STATIC_IP, GATEWAY, SUBNET)) {
        Serial.println("Falha ao configurar IP estático");
        return false;
    }
    WiFi.begin(ssid.c_str(), password.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.print("Endereço IP: ");
        Serial.println(WiFi.localIP());
        timeClient.begin();
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        registrar_log("Sistema inicializado e conectado ao WiFi.");
        return true;
    } else {
        Serial.println("\nFalha ao conectar ao WiFi. Verifique as credenciais.");
        preferences.begin("cafeteira", false);
        preferences.remove("wifi_ssid");
        preferences.remove("wifi_pass");
        preferences.end();
        ESP.restart();
        return false;
    }
}


void configurar_rotas_api() {
    // Rota da página principal - AGORA COM VERIFICAÇÃO DE TOKEN
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!session_token.isEmpty() && (millis() - session_start_time < SESSION_TIMEOUT_MS)) {
            AsyncWebHeader* cookie = request->getHeader("Cookie");
            if (cookie != nullptr && cookie->value().indexOf("session=" + session_token) != -1) {
                request->send(SPIFFS, "/index.html", "text/html");
                return;
            }
        }
        session_token = "";
        return request->redirect("/login.html");
    });

    // Rota para a página de login
    server.on("/login.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/login.html", "text/html");
    });

    // Rota para processar o login - AGORA GERA E ENVIA O TOKEN
    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        JsonDocument json;
        deserializeJson(json, (const char*)data);
        String password = json["password"];
        if (password == ADMIN_PASSWORD) {
            session_token = generate_token();
            session_start_time = millis();
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"success\":true}");
            response->addHeader("Set-Cookie", "session=" + session_token + "; Path=/");
            request->send(response);
        } else {
            request->send(401, "application/json", "{\"success\":false, \"message\":\"Senha incorreta\"}");
        }
    });

    // Rota de Logout para invalidar o token
    server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
        session_token = "";
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Logout efetuado.");
        response->addHeader("Set-Cookie", "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        request->send(response);
        request->redirect("/login.html");
    });

    // --- Rotas para servir arquivos estáticos (CSS, JS) ---
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/script.js", "text/javascript");
    });
    server.on("/login-script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/login-script.js", "text/javascript");
    });


    // --- Rotas da API ---

    // API para consultar um usuário específico por UID (usado na página de login)
    server.on("/api/usuario", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("uid")) {
            String uid = request->getParam("uid")->value();
            uid.toUpperCase();
            for (int i = 0; i < total_usuarios; i++) {
                if (usuarios[i].uid == uid) {
                    JsonDocument json;
                    json["success"] = true;
                    json["nome"] = usuarios[i].nome;
                    json["uid"] = usuarios[i].uid;
                    json["creditos"] = usuarios[i].creditos;
                    String response;
                    serializeJson(json, response);
                    request->send(200, "application/json", response);
                    return;
                }
            }
            request->send(404, "application/json", "{\"success\":false, \"message\":\"Usuário não encontrado\"}");
        } else {
            request->send(400, "application/json", "{\"success\":false, \"message\":\"Parâmetro UID ausente\"}");
        }
    });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument json;
        json["total_usuarios"] = total_usuarios;
        json["total_cafes_servidos"] = total_cafes_servidos;
        json["sistema_ocupado"] = sistema_ocupado;
        json["ultimo_evento"] = ultimo_evento;
        json["cafes_restantes"] = cafes_restantes;
        json["max_cafes"] = MAX_CAFES;
        String response;
        serializeJson(json, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/usuarios", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument json;
        JsonArray usuarios_json = json["usuarios"].to<JsonArray>();
        for (int i = 0; i < total_usuarios; i++) {
            JsonObject usuario = usuarios_json.add<JsonObject>();
            usuario["uid"] = usuarios[i].uid;
            usuario["nome"] = usuarios[i].nome;
            usuario["creditos"] = usuarios[i].creditos;
        }
        String response;
        serializeJson(json, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/usuarios", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            JsonDocument json;
            if (deserializeJson(json, (const char*)data) != DeserializationError::Ok) {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
                return;
            }
            if (adicionar_usuario(json["uid"], json["nome"])) {
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(409, "application/json", "{\"success\":false, \"message\":\"UID já existe ou limite atingido.\"}");
            }
        });

    server.on("/api/usuarios", HTTP_DELETE, [](AsyncWebServerRequest* request) {}, NULL,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            JsonDocument json;
            if (deserializeJson(json, (const char*)data) != DeserializationError::Ok) {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
                return;
            }
            if (remover_usuario(json["uid"])) {
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(404, "application/json", "{\"success\":false, \"message\":\"UID não encontrado.\"}");
            }
        });

    server.on("/api/servir-cafe", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (!sistema_ocupado && cafes_restantes > 0) {
            servir_cafe("MANUAL", nullptr);
            request->send(200, "application/json", "{\"success\":true}");
        } else if (sistema_ocupado) {
            request->send(423, "application/json", "{\"success\":false, \"message\":\"Sistema ocupado\"}");
        } else {
            request->send(409, "application/json", "{\"success\":false, \"message\":\"Sem café\"}");
        }
    });
}
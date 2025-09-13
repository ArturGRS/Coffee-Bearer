#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <DNSServer.h>
#include "credentials.h"
#include "settings.h" // Must be included to see the extern declarations
#include "persistence.h"
#include "user_management.h"
#include <SPIFFS.h>
#include "feedback.h"
#include "coffee_machine.h"
#include "rfid_handler.h"
#include "web_server.h"

// ============== DEFINIÇÕES PARA VARIÁVEIS GLOBAIS ==============
// Define the variables declared as 'extern' in settings.h
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;


// ============== DEFINIÇÃO DAS VARIÁVEIS GLOBAIS ==============
// (O resto do seu arquivo main.cpp continua aqui)
Preferences preferences;
MFRC522 mfrc522(SS_PIN, RST_PIN);
AsyncWebServer server(80);
DNSServer dnsServer;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
SemaphoreHandle_t xSemaphore = NULL;

Usuario usuarios[MAX_USUARIOS];
int total_usuarios = 0;

unsigned long ultimo_tempo_leitura = 0;
String ultimo_uid_lido = "";

int total_cafes_servidos = 0;
int cafes_restantes = MAX_CAFES;
unsigned long tempo_total_funcionamento = 0;
bool sistema_ocupado = false;
String ultimo_evento = "Sistema inicializado";
unsigned long tempo_ultimo_evento = 0;

unsigned long ultimo_reset_semanal = 0;
unsigned long ultimo_check_reset = 0;

// ============== FUNÇÃO DE RESET DE CRÉDITOS ==============
void resetar_creditos_semanal() {
    if (ultimo_reset_semanal == 0) {
        ultimo_reset_semanal = millis();
        salvar_dados();
        return;
    }

    if (millis() - ultimo_reset_semanal > INTERVALO_RESET_SEMANAL) {
        Serial.println("\n===============================");
        Serial.println("     RESET SEMANAL DE CRÉDITOS     ");
        Serial.println("===============================");

        for (int i = 0; i < total_usuarios; i++) {
            usuarios[i].creditos = CREDITOS_INICIAIS;
        }

        ultimo_reset_semanal = millis();
        registrar_log("Reset semanal de creditos executado para todos os usuarios.");
        salvar_dados();
        som_dados_salvos();

        Serial.println("Créditos de todos os usuários foram resetados para " + String(CREDITOS_INICIAIS));
    }
}

// ============== FUNÇÕES DE COMANDO SERIAL ==============
void processar_comando_serial() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        String original_command = command;
        command.toLowerCase();

        if (command == "help") {
            Serial.println("\n===== COMANDOS DISPONÍVEIS =====");
            Serial.println("add <uid> <nome> - Adiciona um novo usuário");
            Serial.println("rm <uid> - Remove um usuário pelo UID");
            Serial.println("list - Lista todos os usuários cadastrados");
            Serial.println("clear - Apaga todos os dados (usuários e estatísticas)");
            Serial.println("stats - Mostra as estatísticas do sistema");
            Serial.println("servir - Serve um café manualmente");
            Serial.println("logs - Exibe o log de eventos");
            Serial.println("reset - Força reset dos créditos");
            Serial.println("==============================\n");
        }
        else if (command.startsWith("add ")) {
            int firstSpace = original_command.indexOf(' ');
            if (firstSpace != -1) {
                String uid = original_command.substring(firstSpace + 1);
                int secondSpace = uid.indexOf(' ');

                if (secondSpace != -1) {
                    String nome = uid.substring(secondSpace + 1);
                    uid = uid.substring(0, secondSpace);

                    uid.trim();
                    uid.toUpperCase();
                    nome.trim();

                    if (uid.length() > 0 && nome.length() > 0) {
                        adicionar_usuario(uid, nome);
                    } else {
                        Serial.println("Formato inválido. Use: add <UID> <NOME COMPLETO>");
                    }
                } else {
                    Serial.println("Formato inválido. Falta o nome. Use: add <UID> <NOME COMPLETO>");
                }
            }
        }
        else if (command.startsWith("rm ")) {
            String uid = original_command.substring(3);
            uid.trim();
            uid.toUpperCase();
            remover_usuario(uid);
        }
        else if (command == "list") {
            Serial.println("\n===== LISTA DE USUÁRIOS =====");
            if (total_usuarios == 0) {
                Serial.println("Nenhum usuário cadastrado.");
            } else {
                for (int i = 0; i < total_usuarios; i++) {
                    Serial.print(i + 1);
                    Serial.print(": ");
                    Serial.print(usuarios[i].nome);
                    Serial.print(" - UID: ");
                    Serial.print(usuarios[i].uid);
                    Serial.print(" - Créditos: ");
                    Serial.println(usuarios[i].creditos);
                }
            }
            Serial.println("=============================\n");
        }
        else if (command == "clear") {
            Serial.println("ATENÇÃO: ISSO APAGARÁ TUDO! Digite 'CONFIRMAR' para prosseguir.");
            while (Serial.available() == 0) { delay(100); }
            String confirm = Serial.readStringUntil('\n');
            confirm.trim();
            if (confirm == "CONFIRMAR") {
                limpar_dados();
                som_dados_salvos();
            } else {
                Serial.println("Operação cancelada.");
            }
        }
        else if (command == "stats") {
            Serial.println("\n===== ESTATÍSTICAS DO SISTEMA =====");
            Serial.print("Usuários cadastrados: ");
            Serial.println(total_usuarios);
            Serial.print("Total de cafés servidos: ");
            Serial.println(total_cafes_servidos);
            Serial.print("Cafés restantes: ");
            Serial.print(cafes_restantes);
            Serial.print(" / ");
            Serial.println(MAX_CAFES);
            Serial.println("=================================\n");
        }
        else if(command == "servir"){
            if(!sistema_ocupado && cafes_restantes > 0){
                // Adicione 'nullptr' como segundo argumento
                servir_cafe("SERIAL", nullptr);
            } else if(sistema_ocupado) {
                Serial.println("Sistema ocupado no momento.");
            } else {
                Serial.println("Sem café na garrafa.");
            }
        }
        else if (command == "logs") {
            File file = SPIFFS.open("/datalog.txt", FILE_READ);
            if (!file || file.size() == 0) {
                Serial.println("Nenhum log encontrado.");
                return;
            }
            Serial.println("\n===== LOG DE EVENTOS =====");
            while (file.available()) {
                Serial.write(file.read());
            }
            file.close();
            Serial.println("\n========================\n");
        }
        else if (command == "reset") {
            Serial.println("Forçando reset de créditos...");
            for (int i = 0; i < total_usuarios; i++) {
                usuarios[i].creditos = CREDITOS_INICIAIS;
            }
            ultimo_reset_semanal = millis();
            registrar_log("Reset manual de creditos executado.");
            salvar_dados();
            som_dados_salvos();
            Serial.println("Créditos resetados para todos os usuários!");
        }
        else {
            if (command.length() > 0)
                Serial.println("Comando desconhecido. Digite 'help' para ver a lista de comandos.");
        }
    }
}


// ============== SETUP e LOOP ==============
void setup() {
    Serial.begin(115200);
    while (!Serial); // Garante que o monitor serial esteja pronto

    if (!SPIFFS.begin(true)) {
        Serial.println("Falha ao montar o SPIFFS. O sistema não pode continuar.");
        while (true);
    }

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    SPI.begin();
    mfrc522.PCD_Init();

    Serial.println("\n==================================================");
    Serial.println(" SISTEMA CAFETEIRA RFID - v3.4");
    Serial.println("==================================================");

    carregar_dados();

    preferences.begin("cafeteira", true);
    String saved_ssid = preferences.getString("wifi_ssid", "");
    String saved_pass = preferences.getString("wifi_pass", "");
    preferences.end();

    if (saved_ssid.length() > 0) {
        if (connectToWiFi(saved_ssid, saved_pass)) {
            configurar_rotas_api();
            server.begin();
            Serial.println("Servidor web iniciado!");
        } else {
            // O código para aqui, esperando o reinício.
            return;
        }
    } else {
        startProvisioningMode();
    }

    if (ultimo_reset_semanal == 0) {
        ultimo_reset_semanal = millis();
        salvar_dados();
    }

    xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL) {
        Serial.println("Falha ao criar o semáforo. O sistema pode ficar instável.");
    }

    som_inicializacao();

    Serial.println("\nDigite 'HELP' para ver todos os comandos");
    Serial.println("Ou acesse a interface web: http://" + WiFi.localIP().toString());
    Serial.println("\nSistema ativo - aproxime uma tag RFID...");

    tempo_ultimo_evento = millis();
}

void loop() {
    processar_comando_serial();

    if (millis() - ultimo_check_reset > INTERVALO_CHECK_RESET) {
        resetar_creditos_semanal();
        ultimo_check_reset = millis();
    }

    if (sistema_ocupado || !mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        delay(50); // Pequeno delay para evitar sobrecarga da CPU
        return;
    }

    processar_tag_detectada();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}
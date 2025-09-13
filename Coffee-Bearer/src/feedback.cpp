#include "feedback.h"
#include <SPIFFS.h>
#include <NTPClient.h>
#include "time.h"
#include "settings.h" // Para BUZZER_PIN

// Variáveis globais necessárias
extern NTPClient timeClient;

// ============== FUNÇÃO DE LOG ==============
void registrar_log(String evento) {
    if (!SPIFFS.exists("/datalog.txt")) {
        Serial.println("Criando arquivo de log...");
        File file = SPIFFS.open("/datalog.txt", FILE_WRITE);
        if (!file) {
            Serial.println("Falha ao criar arquivo de log");
            return;
        }
        file.println("======== LOG DE EVENTOS DA CAFETEIRA ========");
        file.close();
    }

    File file = SPIFFS.open("/datalog.txt", FILE_APPEND);
    if (!file) {
        Serial.println("Falha ao abrir arquivo de log para escrita");
        return;
    }

    timeClient.update();
    String timestamp = timeClient.getFormattedTime();
    struct tm timeinfo;
    String log_entry;

    if (getLocalTime(&timeinfo)) {
        char dateString[20];
        strftime(dateString, sizeof(dateString), "%d/%m/%Y", &timeinfo);
        log_entry = String(dateString) + " " + timestamp + " - " + evento;
    } else {
        // Se o NTP não estiver sincronizado, use millis() como fallback
        log_entry = "T:" + String(millis() / 1000) + "s - " + evento;
    }
    
    file.println(log_entry);
    Serial.println("LOG: " + log_entry);
    file.close();
}

// ============== FUNÇÕES DE SOM ==============
void som_autorizado() {
    tone(BUZZER_PIN, 1200, 80);
    delay(100);
    tone(BUZZER_PIN, 1500, 80);
}

void som_negado() {
    tone(BUZZER_PIN, 200, 400);
}

void som_inicializacao() {
    tone(BUZZER_PIN, 800, 60);
    delay(80);
    tone(BUZZER_PIN, 1000, 60);
    delay(80);
    tone(BUZZER_PIN, 1200, 60);
}

void som_cafe_pronto() {
    tone(BUZZER_PIN, 1300, 50);
    delay(60);
    tone(BUZZER_PIN, 1300, 50);
    delay(60);
    tone(BUZZER_PIN, 1300, 50);
    delay(60);
    tone(BUZZER_PIN, 1600, 100);
}

void som_dados_salvos() {
    tone(BUZZER_PIN, 1800, 50);
    delay(60);
    tone(BUZZER_PIN, 1800, 50);
}

void som_sem_cafe() {
    tone(BUZZER_PIN, 440, 150);
    delay(160);
    tone(BUZZER_PIN, 440, 150);
    delay(160);
    tone(BUZZER_PIN, 440, 150);
}

void som_reabastecido() {
    tone(BUZZER_PIN, 1500, 80);
    delay(100);
    tone(BUZZER_PIN, 1800, 80);
    delay(100);
    tone(BUZZER_PIN, 2200, 120);
}
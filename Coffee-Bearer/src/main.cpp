/*
==================================================
SISTEMA CAFETEIRA RFID - v3.3 - VERSÃO LIMPA
Controle de Nível, Chave Mestra e Reset Semanal de Créditos
==================================================
*/

// ================== INCLUDES E DEFINIÇÕES ==================
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "SPIFFS.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"
#include "credentials.h"

#include "hardware.h"
#include "usuarios.h"
#include "persistencia.h"
#include "controle.h"
#include "api.h"
#include "rfid.h"
#include "log.h"
#include "som.h"
#include "serialcmd.h"


// ================== SETUP E LOOP ==================
void setup() {
    Serial.begin(115200);

    if(!SPIFFS.begin(true)){
        Serial.println("ERRO FATAL: Ocorreu um erro ao montar o SPIFFS. O sistema será paralisado.");
        // Pisca um LED ou emite um som de erro contínuo para indicar falha
        while(1){
            digitalWrite(BUZZER_PIN, HIGH); delay(100);
            digitalWrite(BUZZER_PIN, LOW); delay(100);
        }
    }

    pinMode(BUZZER_PIN, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    SPI.begin();
    mfrc522.PCD_Init();

    Serial.println("\n==================================================");
    Serial.println(" SISTEMA CAFETEIRA RFID - v3.3");
    Serial.println("==================================================");

    carregar_dados();
    inicializar_wifi();

    if (WiFi.status() == WL_CONNECTED) {
        configurar_rotas_api();
        server.begin();
        Serial.println("Servidor web iniciado!");
    }

    if(ultimo_reset_semanal == 0) { // Garante que a data do primeiro reset seja salva
        ultimo_reset_semanal = millis();
        salvar_dados();
    }

     xSemaphore = xSemaphoreCreateMutex();
      if (xSemaphore == NULL) {
          Serial.println("ERRO: Não foi possível criar o semáforo!");
          // Trava a execução se não conseguir criar o semáforo, pois é crítico
          while(1); 
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

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi desconectado. Tentando reconectar...");
        WiFi.reconnect();
        delay(2000);
    }

    if (sistema_ocupado || !mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    processar_tag_detectada();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}
#include "coffee_machine.h"
#include "credentials.h"
#include "settings.h"
#include "feedback.h"
#include "persistence.h"

extern SemaphoreHandle_t xSemaphore;
extern bool sistema_ocupado;
extern String ultimo_evento;
extern unsigned long tempo_ultimo_evento;
extern int total_cafes_servidos;
extern int cafes_restantes;

void servir_cafe(String nome_usuario, int* creditos_ptr) {
    if (xSemaphore == NULL || xSemaphoreTake(xSemaphore, (TickType_t)10) != pdTRUE) {
        Serial.println("ERRO DE CONCORRÊNCIA: Acesso bloqueado pelo semáforo.");
        return;
    }

    sistema_ocupado = true;
    ultimo_evento = "Servindo café para " + nome_usuario;
    tempo_ultimo_evento = millis();

    Serial.println("\n===============================");
    Serial.println(" SERVINDO CAFÉ PARA " + nome_usuario);
    Serial.println("===============================");

    digitalWrite(RELAY_PIN, HIGH);
    for (int i = TEMPO_CAFE_MS / 1000; i > 0; i--) {
        Serial.print(" Servindo... ");
        Serial.print(i);
        Serial.println(" segundos");
        delay(1000);
    }
    digitalWrite(RELAY_PIN, LOW);
    Serial.println(" Café servido com sucesso!");
    som_cafe_pronto();

    if (creditos_ptr != nullptr && nome_usuario != "MANUAL" && nome_usuario != "SERIAL") {
        (*creditos_ptr)--;
    }

    total_cafes_servidos++;
    cafes_restantes--;

    String log_msg = "Cafe servido para " + nome_usuario;
    if (creditos_ptr != nullptr) {
        log_msg += ". Creditos restantes: " + String(*creditos_ptr);
    }
    log_msg += ". Cafes na garrafa: " + String(cafes_restantes);

    registrar_log(log_msg);
    salvar_dados();

    ultimo_evento = "Café servido para " + nome_usuario;
    tempo_ultimo_evento = millis();
    sistema_ocupado = false;

    xSemaphoreGive(xSemaphore);

    Serial.println("\nAguardando próxima tag...");
}

void reabastecer_cafe() {
    Serial.println("\n===============================");
    Serial.println("      TAG MESTRA DETECTADA     ");
    Serial.println("===============================");
    cafes_restantes = MAX_CAFES;
    ultimo_evento = "Garrafa de café reabastecida!";
    tempo_ultimo_evento = millis();
    registrar_log("Garrafa de cafe reabastecida via tag mestra.");
    salvar_dados();
    som_reabastecido();
    Serial.println("Cafeteira reabastecida. Nível: " + String(cafes_restantes));
}
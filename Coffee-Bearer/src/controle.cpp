#include "controle.h"
#include "hardware.h"
#include "log.h"
#include "usuarios.h"
#include <Arduino.h>
#include "som.h"

bool sistema_ocupado = false;
int cafes_restantes = MAX_CAFES;
int total_cafes_servidos = 0;
unsigned long tempo_total_funcionamento = 0;
unsigned long tempo_ultimo_evento = 0;
String ultimo_evento = "";
unsigned long ultimo_reset_semanal = 0;
unsigned long ultimo_check_reset = 0;
SemaphoreHandle_t xSemaphore = nullptr;

void controle_init() {
    cafes_restantes = MAX_CAFES;
    total_cafes_servidos = 0;
    tempo_total_funcionamento = 0;
    tempo_ultimo_evento = millis();
    ultimo_evento = "Sistema iniciado";
    ultimo_reset_semanal = 0;
    ultimo_check_reset = 0;
    xSemaphore = nullptr;
}

void controle_loop() {
    // Placeholder para lógica de controle periódico
}

void servir_cafe(String nome_usuario, int* creditos_ptr) {
	sistema_ocupado = true;
	Serial.println("\n===============================");
	Serial.println(" SERVINDO CAFÉ PARA " + nome_usuario);
	Serial.println("===============================");
	digitalWrite(RELAY_PIN, HIGH);
	delay(3000); // Simula tempo de servir café
	digitalWrite(RELAY_PIN, LOW);
	Serial.println(" Café servido com sucesso!");
	// Decrementa créditos se aplicável
	if (creditos_ptr != nullptr && nome_usuario != "MANUAL" && nome_usuario != "SERIAL") {
		(*creditos_ptr)--;
	}
	sistema_ocupado = false;
}

void resetar_creditos_semanal() {
    // Implemente a lógica de reset semanal aqui (ver código original)
}

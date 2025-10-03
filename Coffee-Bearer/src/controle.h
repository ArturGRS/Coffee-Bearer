#ifndef CONTROLE_H
#define CONTROLE_H

#include <Arduino.h>
#include "usuarios.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define TEMPO_CAFE_MS 6000
#define MAX_CAFES 15
#define INTERVALO_CHECK_RESET 60000UL
#define INTERVALO_RESET_SEMANAL 604800000UL

extern bool sistema_ocupado;
extern int cafes_restantes;
extern int total_cafes_servidos;
extern unsigned long tempo_total_funcionamento;
extern unsigned long tempo_ultimo_evento;
extern String ultimo_evento;
extern unsigned long ultimo_reset_semanal;
extern unsigned long ultimo_check_reset;
extern SemaphoreHandle_t xSemaphore;

void controle_init();
void controle_loop();
void servir_cafe(String nome_usuario, int* creditos_ptr);
void resetar_creditos_semanal();

#endif // CONTROLE_H

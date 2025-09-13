#pragma once

// ============== CONFIGURAÇÕES DOS PINOS ==============
#define RST_PIN 4
#define SS_PIN 5
#define BUZZER_PIN 15
#define RELAY_PIN 13

// ============== CONFIGURAÇÕES DA CHAVE MESTRA E RESET ==============
#define INTERVALO_RESET_SEMANAL (7 * 24 * 60 * 60 * 1000UL)
#define INTERVALO_CHECK_RESET 3600000UL

// ============== CONFIGURAÇÕES WIFI E HORA (NTP) ==============
// Declare variables as extern to prevent multiple definitions
extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// Defina seu IP fixo aqui
#define STATIC_IP IPAddress(192, 168, 1, 150)
#define GATEWAY   IPAddress(192, 168, 1, 1)
#define SUBNET    IPAddress(255, 255, 255, 0)
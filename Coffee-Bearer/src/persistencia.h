#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

extern const char* ntpServer;
extern long gmtOffset_sec;
extern int daylightOffset_sec;
extern NTPClient timeClient;

void inicializar_wifi();

#endif // PERSISTENCIA_H

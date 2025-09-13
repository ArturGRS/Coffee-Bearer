#pragma once
#include <Arduino.h>

void startProvisioningMode();
bool connectToWiFi(String ssid, String password);
void configurar_rotas_api();
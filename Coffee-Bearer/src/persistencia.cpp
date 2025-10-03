#include "persistencia.h"
#include "credentials.h"
#include "log.h"

const char* ntpServer = "pool.ntp.org";
long gmtOffset_sec = -10800;
int daylightOffset_sec = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, 60000);

void inicializar_wifi() {
	Serial.print("Conectando a ");
	Serial.println(WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWiFi conectado!");
	Serial.print("Endereço IP: ");
	Serial.println(WiFi.localIP());
	// log opcional
	registrar_log("Sistema inicializado e conectado ao WiFi.");
	timeClient.begin();
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

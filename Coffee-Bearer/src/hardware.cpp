#include "hardware.h"

void hardware_init() {
	pinMode(BUZZER_PIN, OUTPUT);
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);
	SPI.begin();
}

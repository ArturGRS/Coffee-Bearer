#include "serialcmd.h"
#include <Arduino.h>
#include "usuarios.h"

void processar_comando_serial() {
	if (Serial.available() > 0) {
		String command = Serial.readStringUntil('\n');
		command.trim();
		if (command == "list") {
			for (int i = 0; i < total_usuarios; i++) {
				Serial.print(usuarios[i].nome);
				Serial.print(" - UID: ");
				Serial.print(usuarios[i].uid);
				Serial.print(" - Créditos: ");
				Serial.println(usuarios[i].creditos);
			}
		}
	}
}

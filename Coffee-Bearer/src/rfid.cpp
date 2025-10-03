#include "rfid.h"
#include "controle.h"
#include "usuarios.h"
#include "som.h"

MFRC522 mfrc522;

void rfid_init() {
	mfrc522.PCD_Init();
}

void processar_tag_detectada() {
	String uid = "";
	for (byte i = 0; i < mfrc522.uid.size; i++) {
		uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
		uid += String(mfrc522.uid.uidByte[i], HEX);
	}
	uid.toUpperCase();
	int idx = verificar_autorizacao(uid);
	if (idx >= 0) {
		som_autorizado();
		servir_cafe(usuarios[idx].nome, &usuarios[idx].creditos);
	} else {
		som_negado();
	}
}

#include "rfid_handler.h"
#include <MFRC522.h>
#include "credentials.h" // Para COOLDOWN_MS, UID_MESTRE
#include "user_management.h"
#include "coffee_machine.h"
#include "feedback.h"

// Variáveis globais necessárias
extern MFRC522 mfrc522;
extern String ultimo_uid_lido;
extern unsigned long ultimo_tempo_leitura;
extern String ultimo_evento;
extern unsigned long tempo_ultimo_evento;
extern int cafes_restantes;

// ============== FUNÇÕES DE RFID ==============
String ler_uid_tag() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        uid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    uid.toUpperCase();
    uid.trim();
    return uid;
}

void processar_tag_detectada() {
    String uid = ler_uid_tag();
    unsigned long tempo_atual = millis();

    if (uid == ultimo_uid_lido && (tempo_atual - ultimo_tempo_leitura) < COOLDOWN_MS) {
        return;
    }

    ultimo_uid_lido = uid;
    ultimo_tempo_leitura = tempo_atual;

    Serial.println("\n===============================");
    Serial.println(" TAG DETECTADA: " + uid);
    Serial.println("===============================");

    // 1. Checar se é a tag mestra
    if (uid == UID_MESTRE) {
        reabastecer_cafe();
        return; // Finaliza o processamento aqui
    }

    // 2. Checar se há café na garrafa
    if (cafes_restantes <= 0) {
        Serial.println(" ACESSO NEGADO - SEM CAFÉ NA GARRAFA!");
        ultimo_evento = "Tentativa de uso sem café na garrafa.";
        tempo_ultimo_evento = millis();
        som_sem_cafe();
        return; // Finaliza o processamento aqui
    }

    // 3. Processar como usuário normal
    int usuario_index = verificar_autorizacao(uid);

    if (usuario_index >= 0) {
        Serial.println(" ACESSO AUTORIZADO");
        som_autorizado();
        servir_cafe(usuarios[usuario_index].nome, &usuarios[usuario_index].creditos);
    } else {
        Serial.println(" ACESSO NEGADO");
        if (usuario_index == -2) {
            // Usuário sem créditos - som já foi tocado na função verificar_autorizacao
        } else {
            ultimo_evento = "Acesso negado - UID não cadastrado: " + uid;
            tempo_ultimo_evento = millis();
        }
        som_negado();
    }
}
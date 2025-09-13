#include "user_management.h"
#include "credentials.h" // Para MAX_USUARIOS, CREDITOS_INICIAIS
#include "persistence.h" // Para salvar_dados()
#include "feedback.h"    // Para registrar_log()

extern String ultimo_evento;
extern unsigned long tempo_ultimo_evento;

bool adicionar_usuario(String uid, String nome) {
    if (total_usuarios >= MAX_USUARIOS) {
        Serial.println("Erro: Limite máximo de usuários atingido!");
        return false;
    }
    
    for (int i = 0; i < total_usuarios; i++) {
        if (usuarios[i].uid == uid) {
            Serial.println("UID já cadastrado para: " + usuarios[i].nome);
            return false;
        }
    }

    usuarios[total_usuarios].uid = uid;
    usuarios[total_usuarios].nome = nome;
    // CORRIGIDO AQUI
    usuarios[total_usuarios].creditos = CREDITOS_INICIAIS; 
    total_usuarios++;

    // E CORRIGIDO AQUI
    Serial.println("Usuário adicionado: " + nome + " com " + String(CREDITOS_INICIAIS) + " créditos."); 
    registrar_log("Usuario adicionado: " + nome + " (UID: " + uid + ")");
    salvar_dados();

    ultimo_evento = "Usuário adicionado: " + nome;
    tempo_ultimo_evento = millis();

    return true;
}

bool remover_usuario(String uid) {
    for (int i = 0; i < total_usuarios; i++) {
        if (usuarios[i].uid == uid) {
            String nome = usuarios[i].nome;
            for (int j = i; j < total_usuarios - 1; j++) {
                usuarios[j] = usuarios[j + 1];
            }
            total_usuarios--;
            // Limpa completamente a memória da última posição do array
            memset(&usuarios[total_usuarios], 0, sizeof(Usuario));

            Serial.println("Usuário removido: " + nome);
            registrar_log("Usuario removido: " + nome + " (UID: " + uid + ")");
            salvar_dados();

            ultimo_evento = "Usuário removido: " + nome;
            tempo_ultimo_evento = millis();

            return true;
        }
    }
    Serial.println("UID não encontrado na lista");
    return false;
}

int verificar_autorizacao(String uid) {
    for (int i = 0; i < total_usuarios; i++) {
        if (usuarios[i].uid == uid) {
            if (usuarios[i].creditos > 0) {
                return i; // Autorizado, retorna o índice
            } else {
                Serial.println("ACESSO NEGADO - SEM CREDITOS");
                ultimo_evento = "Acesso negado (sem creditos): " + usuarios[i].nome;
                tempo_ultimo_evento = millis();
                return -2; // Indica que o usuário foi encontrado mas não tem créditos
            }
        }
    }
    return -1; // Não autorizado / não encontrado
}
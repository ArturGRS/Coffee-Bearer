#include "persistence.h"
#include <Preferences.h>
#include "user_management.h"
#include "credentials.h"
#include "feedback.h"

// Variáveis globais definidas em main.cpp
extern Preferences preferences;
extern Usuario usuarios[MAX_USUARIOS];
extern int total_usuarios;
extern int total_cafes_servidos;
extern unsigned long tempo_total_funcionamento;
extern int cafes_restantes;
extern unsigned long ultimo_reset_semanal;


// ============== FUNÇÕES DE PERSISTÊNCIA ==============
void salvar_dados() {
  Serial.println("Salvando dados na memória flash...");
  preferences.begin("cafeteira", false);
  preferences.putInt("total_users", total_usuarios);
  for (int i = 0; i < total_usuarios; i++) {
    String key_uid = "uid_" + String(i);
    String key_nome = "nome_" + String(i);
    String key_cred = "cred_" + String(i);
    preferences.putString(key_uid.c_str(), usuarios[i].uid);
    preferences.putString(key_nome.c_str(), usuarios[i].nome);
    preferences.putInt(key_cred.c_str(), usuarios[i].creditos);
  }
  preferences.putInt("cafes_servidos", total_cafes_servidos);
  preferences.putULong("tempo_funcionamento", tempo_total_funcionamento);
  preferences.putInt("cafes_restantes", cafes_restantes);
  preferences.putULong("ultimo_reset", ultimo_reset_semanal);
  preferences.end();
  Serial.println("Dados salvos com sucesso!");
}

void carregar_dados() {
  Serial.println("Carregando dados da memória flash...");
  preferences.begin("cafeteira", true);
  total_usuarios = preferences.getInt("total_users", 0);
  for (int i = 0; i < total_usuarios; i++) {
    String key_uid = "uid_" + String(i);
    String key_nome = "nome_" + String(i);
    String key_cred = "cred_" + String(i);
    usuarios[i].uid = preferences.getString(key_uid.c_str(), "");
    usuarios[i].nome = preferences.getString(key_nome.c_str(), "");
    usuarios[i].creditos = preferences.getInt(key_cred.c_str(), 0);
  }
  total_cafes_servidos = preferences.getInt("cafes_servidos", 0);
  tempo_total_funcionamento = preferences.getULong("tempo_funcionamento", 0);
  cafes_restantes = preferences.getInt("cafes_restantes", MAX_CAFES);
  ultimo_reset_semanal = preferences.getULong("ultimo_reset", 0);
  preferences.end();
  Serial.print("Carregados ");
  Serial.print(total_usuarios);
  Serial.println(" usuários da memória!");
  Serial.print(cafes_restantes);
  Serial.println(" cafés restantes na garrafa.");
}

void limpar_dados() {
  Serial.println("Limpando todos os dados salvos...");
  preferences.begin("cafeteira", false);
  preferences.clear();
  preferences.end();
  total_usuarios = 0;
  total_cafes_servidos = 0;
  tempo_total_funcionamento = 0;
  cafes_restantes = MAX_CAFES;
  for (int i = 0; i < MAX_USUARIOS; i++) {
    usuarios[i] = {}; // Limpa a struct
  }
  Serial.println("Todos os dados foram apagados!");
  registrar_log("TODOS OS DADOS FORAM APAGADOS");
}
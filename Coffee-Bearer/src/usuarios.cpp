#include "usuarios.h"
#include <Preferences.h>
#include "log.h"

Usuario usuarios[MAX_USUARIOS];
int total_usuarios = 0;

void salvar_dados() {
	Preferences preferences;
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
	preferences.end();
}

void carregar_dados() {
	Preferences preferences;
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
	preferences.end();
}

void limpar_dados() {
	Preferences preferences;
	preferences.begin("cafeteira", false);
	preferences.clear();
	preferences.end();
	total_usuarios = 0;
	for (int i = 0; i < MAX_USUARIOS; i++) {
		usuarios[i] = {};
	}
}

bool adicionar_usuario(String uid, String nome) {
		if (total_usuarios >= MAX_USUARIOS) return false;
		for (int i = 0; i < total_usuarios; i++) {
				if (usuarios[i].uid == uid) return false;
		}
		usuarios[total_usuarios].uid = uid;
		usuarios[total_usuarios].nome = nome;
		usuarios[total_usuarios].creditos = CREDITOS_INICIAIS;
		total_usuarios++;
		salvar_dados();
		return true;
}

bool remover_usuario(String uid) {
		for (int i = 0; i < total_usuarios; i++) {
				if (usuarios[i].uid == uid) {
						for (int j = i; j < total_usuarios - 1; j++) {
								usuarios[j] = usuarios[j + 1];
						}
						total_usuarios--;
						usuarios[total_usuarios] = {};
						salvar_dados();
						return true;
				}
		}
		return false;
}

int verificar_autorizacao(String uid) {
		for (int i = 0; i < total_usuarios; i++) {
				if (usuarios[i].uid == uid) {
						if (usuarios[i].creditos > 0) return i;
						else return -2;
				}
		}
		return -1;
}

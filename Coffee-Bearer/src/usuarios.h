#ifndef USUARIOS_H
#define USUARIOS_H

#include <Arduino.h>

#define MAX_USUARIOS 50
#define CREDITOS_INICIAIS 7

struct Usuario {
	String uid;
	String nome;
	int creditos;
};

extern Usuario usuarios[MAX_USUARIOS];
extern int total_usuarios;

void carregar_dados();
void salvar_dados();
void limpar_dados();
bool adicionar_usuario(String uid, String nome);
bool remover_usuario(String uid);
int verificar_autorizacao(String uid);

#endif // USUARIOS_H

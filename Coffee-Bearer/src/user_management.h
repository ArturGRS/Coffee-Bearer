// include/user_management.h

#pragma once
#include <Arduino.h>
#include "credentials.h" // Para MAX_USUARIOS

// 1. Definição da Estrutura do Usuário
struct Usuario {
    String uid;
    String nome;
    int creditos;
};

// 2. Declaração da variável global como externa
extern Usuario usuarios[MAX_USUARIOS];
extern int total_usuarios;

// Protótipos das funções
bool adicionar_usuario(String uid, String nome);
bool remover_usuario(String uid);
int verificar_autorizacao(String uid);
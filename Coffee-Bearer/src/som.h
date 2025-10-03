#ifndef SOM_H
#define SOM_H

#include <Arduino.h>
#include "hardware.h"

void som_inicializacao();
void som_autorizado();
void som_negado();
void som_cafe_pronto();
void som_dados_salvos();
void som_sem_cafe();
void som_reabastecido();

#endif // SOM_H

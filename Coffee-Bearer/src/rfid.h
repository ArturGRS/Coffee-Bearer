#ifndef RFID_H
#define RFID_H

#include <MFRC522.h>
#include <Arduino.h>

extern MFRC522 mfrc522;
void rfid_init();
void processar_tag_detectada();

#endif // RFID_H

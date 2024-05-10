#ifndef UMIDADE_HELPER_H
#define UMIDADE_HELPER_H

#include <Arduino.h>
#include "LinkedList.h" // Inclua o cabeçalho LinkedList.h aqui

extern LinkedList listaUmidades; // Declare listaUmidades como externa

#define PINO_UMIDADE 33
#define NUM_LEITURAS 10
#define UMIDADE_MINIMA 30

int lerUmidade();
void enviarUmidadeMQTT(int umidade, PubSubClient& mqttClient);


#endif

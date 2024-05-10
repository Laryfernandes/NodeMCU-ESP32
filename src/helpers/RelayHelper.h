#ifndef RELAY_HELPER_H
#define RELAY_HELPER_H

#include <Arduino.h>
#include <PubSubClient.h>

#define PINO_RELE 32

void inicializarRelay(PubSubClient& mqttClient); // Modificado para aceitar um objeto PubSubClient
void acionarRelay(PubSubClient& mqttClient);
void desligarRelay(PubSubClient& mqttClient);

#endif

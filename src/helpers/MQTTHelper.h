#ifndef MQTT_HELPER_H
#define MQTT_HELPER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Declarando a variável bloqueador como extern
extern bool bloqueador;
extern PubSubClient mqttClient; // Declaração externa do mqttClient


void connectMQTT();
void callback(char* topic, byte* payload, unsigned int length); // Declaração da função callback

void acenderLedVirtual();
void desligarLedVirtual();

#endif

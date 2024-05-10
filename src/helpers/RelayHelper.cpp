#include "RelayHelper.h"
#include "MQTTHelper.h"

void inicializarRelay(PubSubClient& mqttClient) {
  pinMode(PINO_RELE, OUTPUT);
  desligarRelay(mqttClient);
}


void publicarMensagemRelayAcionado(PubSubClient& mqttClient) {
  mqttClient.publish("status/relay", "Acionado");
}

void publicarMensagemRelayDesligado(PubSubClient& mqttClient) {
  mqttClient.publish("status/relay", "Desligado");
}


void acionarRelay(PubSubClient& mqttClient) {
  digitalWrite(PINO_RELE, HIGH);
  publicarMensagemRelayAcionado(mqttClient);
}

void desligarRelay(PubSubClient& mqttClient) {
  digitalWrite(PINO_RELE, LOW);
  publicarMensagemRelayDesligado(mqttClient);
}

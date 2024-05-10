#include "UmidadeHelper.h"
#include "MQTTHelper.h"

int lerUmidade() {
  int somaLeituras = 0;
  for (int i = 0; i < NUM_LEITURAS; i++) {
    somaLeituras += analogRead(PINO_UMIDADE);
    delay(10);
  }

  int umidadePercentual = map(somaLeituras / NUM_LEITURAS, 0, 4095, 100, 0);
  return umidadePercentual;
}


void enviarUmidadeMQTT(int umidade, PubSubClient& mqttClient) {
  // Crie uma string para armazenar a umidade como payload
  String payload = String(umidade);
  // Publique a umidade no tópico desejado
  mqttClient.publish("umidade", payload.c_str());
}
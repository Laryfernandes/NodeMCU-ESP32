#include "MQTTHelper.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "RelayHelper.h"
#include "UmidadeHelper.h" // Inclua o cabeçalho UmidadeHelper.h



const char* MQTT_SERVER = "broker.emqx.io";
const int MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectMQTT() {
  Serial.println("Conectando ao servidor MQTT...");
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Cliente")) {
      Serial.println("Conectado ao broker MQTT");
    } else {
      Serial.print("Falha na conexão, código de retorno=");
      Serial.print(mqttClient.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Mensagem MQTT recebida:");
  Serial.print("Tópico: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.write(payload, length);
  Serial.println();

  // Verifica se a mensagem recebida é para acionar o botão de pressão virtual
  if (strcmp(topic, "botao_pressao") == 0) {
    // Verifica o conteúdo da mensagem
    if (payload[0] == '1') {
      // Se o payload for '1', desbloqueia o bloqueador
      bloqueador = false;
      Serial.println("Bloqueador desbloqueado");
      listaUmidades.limparLista(); // Limpa a lista de umidades
      Serial.println("Lista de umidades zerada após desbloqueio.");
    }
  }
}


void acenderLedVirtual() {
  mqttClient.publish("led_virtual", "ligar"); // Publica uma mensagem para acender o LED virtual
}

void desligarLedVirtual() {
  mqttClient.publish("led_virtual", "desligar"); // Publica uma mensagem para acender o LED virtual
}

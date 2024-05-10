#include <Arduino.h>
#include "helpers/WiFiHelper.h"
#include "helpers/MQTTHelper.h"
#include "helpers/UmidadeHelper.h"
#include "helpers/RelayHelper.h"
#include "helpers/LinkedList.h"


// Declarando a variável de bloqueio
bool bloqueador = false;
bool limita_mensagem = false;


LinkedList listaUmidades; // Instância da lista encadeada

void setup() {
  Serial.begin(9600);
  connectWiFi();
  connectMQTT();
  mqttClient.subscribe("botao_pressao"); // Inscreva-se no tópico "botao_pressao"
  inicializarRelay(mqttClient); // Inicializa o relé
  pinMode(PINO_UMIDADE, INPUT);
  mqttClient.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); // Reconectar ao WiFi, se desconectado
  }

  if (!mqttClient.connected()) {
    connectMQTT(); // Reconectar ao servidor MQTT, se desconectado
  }
  
  mqttClient.loop(); // Manter a conexão MQTT ativa

  int umidadePercentual = lerUmidade();

  // Verificar se a leitura da umidade foi bem-sucedida
  if (umidadePercentual >= 0 && umidadePercentual <= 100) {
    Serial.print("Umidade: ");
    Serial.print(umidadePercentual);
    Serial.println("%");
    
    // Enviar umidade para o Broker MQTT
    enviarUmidadeMQTT(umidadePercentual, mqttClient);
    
    if (umidadePercentual <= UMIDADE_MINIMA && !bloqueador) {
      acionarRelay(mqttClient); // Aciona o relé
      Serial.println("Relé acionado");
      listaUmidades.adicionarUmidade(umidadePercentual, bloqueador, mqttClient);

      // Armazenar a umidade atual na lista
      listaUmidades.imprimirUmidadeArmazenada(); // Imprime a lista de umidades armazenadas

      // Publicar uma mensagem para acender o LED de desbloqueio
      if (!bloqueador && !limita_mensagem) {
        mqttClient.publish("status/reservatorio", "Normal - Bomba Desbloquada");
        limita_mensagem = true;

      }

      // Publicar uma mensagem para apagar o LED de bloqueio
      if (bloqueador) {
        mqttClient.publish("status/reservatorio", "Vazio - Bomba Bloqueada");
        limita_mensagem = false;
      }
    } else {
      desligarRelay(mqttClient); // Desliga o relé
      Serial.println("Relé desligado");

    }

  } else {
    Serial.println("Erro ao ler umidade. Tentando novamente...");
  }

  delay(1000);
}

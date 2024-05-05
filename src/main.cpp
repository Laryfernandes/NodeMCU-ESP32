#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definição das credenciais da rede Wi-Fi
const char* ssid = "NOME_DA_REDE"; // Substitua pelo nome da sua rede Wi-Fi
const char* password = "SENHA_DA_REDE"; // Substitua pela senha da sua rede Wi-Fi

// Configurações do servidor MQTT (broker)
const char* mqtt_server = "broker.emqx.io"; // Endereço do servidor MQTT
const int mqtt_port = 1883;  // Porta padrão MQTT

WiFiClient espClient; // Cria uma instância do cliente WiFi
PubSubClient client(espClient); // Cria uma instância do PubSubClient para comunicação MQTT

#define sensor 33
#define humidityTopic "umidade/solo" // Tópico utilizado para publicar a umidade do solo

void setup() {
  Serial.begin(9600); // Inicializa a comunicação serial
  
  // Conexão à rede Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP()); // Exibe o endereço IP atribuído ao dispositivo
  
  // Configura o servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void sendHumidity() {
  // Leitura do sensor de umidade
  int value = analogRead(sensor);
  // Mapeamento do valor lido para uma faixa de 0 a 100
  value = map(value, 0, 4095, 0, 100);
  // Inversão do valor para uma escala de 0 a 100
  value = (value - 100) * -1;
  Serial.print("Umidade: ");
  Serial.print(value);
  Serial.println("%");

  // Verifica se a conexão com o broker MQTT está estabelecida
  if (client.connected()) {
    // Publica a umidade no tópico especificado apenas se a conexão estiver ativa
    client.publish(humidityTopic, String(value).c_str());
    Serial.println("Umidade enviada com sucesso");
  } else {
    Serial.println("Erro: Não foi possível enviar a umidade, conexão MQTT não estabelecida");
  }
}

void loop() {
  // Verifica se o cliente MQTT está conectado
  if (!client.connected()) {
    Serial.println("Tentando conexão MQTT...");
    // Tenta se reconectar ao broker MQTT
    if (client.connect("ESP32Cliente")) {
      Serial.println("Conectado ao broker MQTT");
    } else {
      // Exibe mensagem de erro em caso de falha na conexão
      Serial.print("Falha na conexão, código de retorno=");
      Serial.print(client.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000); // Espera 5 segundos antes de tentar novamente
      return;
    }
  }
  client.loop(); // Mantém a comunicação com o broker MQTT ativa
  sendHumidity(); // Envia a leitura da umidade do solo
  delay(1000); // Aguarda 1 segundo antes de realizar uma nova leitura
}

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definição das credenciais da rede Wi-Fi
const char* ssid = "NGNL"; // Substitua pelo nome da sua rede Wi-Fi
const char* password = "Liso9445"; // Substitua pela senha da sua rede Wi-Fi

// Configurações do servidor MQTT (broker)
const char* mqtt_server = "broker.emqx.io"; // Endereço do servidor MQTT
const int mqtt_port = 1883;  // Porta padrão MQTT

WiFiClient espClient; // Cria uma instância do cliente WiFi
PubSubClient client(espClient); // Cria uma instância do PubSubClient para comunicação MQTT

#define sensor 33
#define humidityTopic "umidade/solo" // Tópico utilizado para publicar a umidade do solo
#define controlTopic "controle/dispositivo" // Tópico utilizado para controle do dispositivo

bool deviceRunning = true; // Variável para controlar o estado do dispositivo

void callback(char* topic, byte* payload, unsigned int length) {
  // Converte o payload para uma string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Verifica se a mensagem recebida é para controlar o dispositivo
  if (String(topic) == controlTopic) {
    if (message == "ligar") {
      deviceRunning = true; // Ligar o dispositivo
      Serial.println("Dispositivo ligado");
    } else if (message == "desligar") {
      deviceRunning = false; // Desligar o dispositivo
      Serial.println("Dispositivo desligado");
    }
  }
}

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
  client.setCallback(callback); // Define a função de callback para receber mensagens MQTT
}

void sendHumidity() {
  // Verifica se o dispositivo está ligado antes de enviar a leitura da umidade
  if (deviceRunning) {
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
  } else {
    Serial.println("Dispositivo desligado, não é possível enviar a umidade");
  }
}

void reconnect() {
  // Loop até que estejamos reconectados
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    // Tenta conectar
    if (client.connect("ESP32Cliente")) {
      Serial.println("Conectado ao broker MQTT");
      // Inscreva-se no tópico de controle
      client.subscribe(controlTopic);
    } else {
      Serial.print("Falha na conexão, código de retorno=");
      Serial.print(client.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000); // Espera 5 segundos antes de tentar novamente
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Mantém a comunicação com o broker MQTT ativa
  sendHumidity(); // Envia a leitura da umidade do solo
  delay(1000); // Aguarda 1 segundo antes de realizar uma nova leitura
}

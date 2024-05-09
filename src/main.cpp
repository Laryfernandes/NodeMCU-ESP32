#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definição das credenciais da rede Wi-Fi
const char* ssid = "NGNL"; // Nome da rede Wi-Fi
const char* password = "Liso9445"; // Senha da rede Wi-Fi

// Configurações do servidor MQTT (broker)
const char* mqtt_server = "broker.emqx.io"; // Endereço do servidor MQTT
const int mqtt_port = 1883;  // Porta padrão MQTT

WiFiClient espClient; // Cliente WiFi
PubSubClient mqttClient(espClient); // Cliente MQTT

// Definições dos pinos e tópicos MQTT
#define SOIL_SENSOR_PIN 33
#define HUMIDITY_TOPIC "umidade/solo" // Tópico da umidade do solo
#define CONTROL_TOPIC "controle/dispositivo" // Tópico de controle do dispositivo
#define RELAY_STATUS_TOPIC "status/relé" // Tópico do status do relé

#define RELAY_PIN 32
#define MIN_HUMIDITY 45
#define MAX_HUMIDITY 60

bool deviceRunning = true; // Estado do dispositivo
unsigned long relayActivationTime = 0;
bool relayBlocked = false;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  // Verifica se a mensagem é para controlar o dispositivo
  if (String(topic) == CONTROL_TOPIC) {
    if (message == "ligar") {
      deviceRunning = true; // Liga o dispositivo
      Serial.println("Dispositivo ligado");
    } else if (message == "desligar") {
      deviceRunning = false; // Desliga o dispositivo
      Serial.println("Dispositivo desligado");
    }
  }
}

void setup() {
  Serial.begin(9600); // Inicializa a comunicação serial
  WiFi.begin(ssid, password); // Conecta-se à rede Wi-Fi
  
  // Aguarda até que a conexão seja estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP()); // Exibe o endereço IP atribuído ao dispositivo

  mqttClient.setServer(mqtt_server, mqtt_port); // Configura o servidor MQTT
  mqttClient.setCallback(mqttCallback); // Define a função de callback para mensagens MQTT recebidas
  
  pinMode(RELAY_PIN, OUTPUT); // Configura o pino do relé como saída
  digitalWrite(RELAY_PIN, LOW); // Desliga o relé inicialmente
}

void sendHumidity() {
  if (deviceRunning) { // Verifica se o dispositivo está ligado
    int humidityValue = analogRead(SOIL_SENSOR_PIN); // Realiza a leitura do sensor de umidade
    humidityValue = map(humidityValue, 0, 4095, 0, 100); // Mapeia o valor lido para uma faixa de 0 a 100
    humidityValue = (humidityValue - 100) * -1; // Inverte o valor para uma escala de 0 a 100
    Serial.print("Umidade: ");
    Serial.print(humidityValue);
    Serial.println("%");

    // Controla o relé com base na umidade medida
    if (humidityValue < MIN_HUMIDITY && !relayBlocked) {
      if (millis() - relayActivationTime >= 5000) {
        relayBlocked = true;
        digitalWrite(RELAY_PIN, LOW); // Desliga o relé
        mqttClient.publish(RELAY_STATUS_TOPIC, "bloqueado"); // Publica o status do relé
        Serial.println("Relé bloqueado");
      } else {
        digitalWrite(RELAY_PIN, HIGH); // Liga o relé
        mqttClient.publish(RELAY_STATUS_TOPIC, "ativo"); // Publica o status do relé
        Serial.println("Relé ativado");
      }
    } else if (humidityValue >= MAX_HUMIDITY || relayBlocked) {
      digitalWrite(RELAY_PIN, LOW); // Desliga o relé
      mqttClient.publish(RELAY_STATUS_TOPIC, "inativo"); // Publica o status do relé
      Serial.println("Relé desligado");
    }

    // Verifica se a conexão MQTT está ativa e publica a umidade do solo
    if (mqttClient.connected()) {
      mqttClient.publish(HUMIDITY_TOPIC, String(humidityValue).c_str());
      Serial.println("Umidade enviada com sucesso");
    } else {
      Serial.println("Erro: Não foi possível enviar a umidade, conexão MQTT não estabelecida");
    }
  } else {
    digitalWrite(RELAY_PIN, LOW); // Desliga o relé se o dispositivo estiver desligado
    Serial.println("Dispositivo desligado, não é possível enviar a umidade");
    mqttClient.publish(RELAY_STATUS_TOPIC, "inativo"); // Publica o status do relé
  }
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (mqttClient.connect("ESP32Cliente")) { // Tenta conectar ao broker MQTT
      Serial.println("Conectado ao broker MQTT");
      mqttClient.subscribe(CONTROL_TOPIC); // Inscreve-se no tópico de controle
    } else {
      Serial.print("Falha na conexão, código de retorno=");
      Serial.print(mqttClient.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000); // Espera 5 segundos antes de tentar reconectar
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    mqttReconnect(); // Reconecta ao servidor MQTT se a conexão for perdida
  }
  mqttClient.loop(); // Mantém a comunicação com o servidor MQTT
  sendHumidity(); // Envia a umidade do solo

  if (deviceRunning) {
    int humidityValue = analogRead(SOIL_SENSOR_PIN);
    humidityValue = map(humidityValue, 0, 4095, 0, 100);
    humidityValue = (humidityValue - 100) * -1;
    if (humidityValue >= MIN_HUMIDITY && humidityValue < MAX_HUMIDITY) {
      relayActivationTime = millis(); // Atualiza o tempo de ativação do relé
      relayBlocked = false; // Libera o relé
    }
  }

  delay(1000); // Aguarda 1 segundo antes de realizar uma nova leitura
}

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configurações do Wi-Fi
const char* ssid = "";
const char* password = "";

// Configurações MQTT
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

// Pinagem
#define SOIL_SENSOR_PIN 33
#define RELAY_PIN 32

// Tópicos MQTT
#define HUMIDITY_TOPIC "umidade/solo"
#define CONTROL_TOPIC "controle/dispositivo"
#define RELAY_STATUS_TOPIC "status/rele"
#define RESERVOIR_STATUS_TOPIC "status/reservatorio"
#define UNLOCK_TOPIC "controle/desbloquear"

// Limites de umidade
#define MIN_HUMIDITY 15
#define MAX_HUMIDITY 30

// Variáveis de controle
bool deviceRunning = true;
bool relayBlocked = false;
bool manualUnlocked = false;
unsigned long lastHumidityChangeTime = 0;
unsigned int consecutiveLowReadings = 0;
bool previousRelayState = false;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Protótipos de Função
void setupWiFi();
void setupMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishHumidity(int humidityValue);
void publishRelayStatus(const char* status);
void publishReservoirStatus(bool blocked);
void updateRelayState(int humidityValue);
void handleControlMessage(const String& message);
void handleUnlockMessage(const String& message);
void connectToWiFi();
void connectToMQTT();
int readHumidity();
void sendHumidityData(int humidityValue);

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  connectToMQTT();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
  sendHumidityData(readHumidity());
  delay(1000);
}

void connectToWiFi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  Serial.println("Conectando ao servidor MQTT...");
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Cliente")) {
      Serial.println("Conectado ao broker MQTT");
      mqttClient.subscribe(CONTROL_TOPIC);
      mqttClient.subscribe(UNLOCK_TOPIC);
    } else {
      Serial.print("Falha na conexão, código de retorno=");
      Serial.print(mqttClient.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);
  if (String(topic) == CONTROL_TOPIC) {
    handleControlMessage(message);
  } else if (String(topic) == UNLOCK_TOPIC) {
    handleUnlockMessage(message);
  }
}

void handleControlMessage(const String& message) {
  deviceRunning = (message == "ligar");
  Serial.println(deviceRunning ? "Dispositivo ligado" : "Dispositivo desligado");
}

void handleUnlockMessage(const String& message) {
  if (message == "desbloquear") {
    manualUnlocked = true;
    digitalWrite(RELAY_PIN, HIGH);
    publishRelayStatus("desbloqueado");
    Serial.println("Relé desbloqueado manualmente");
    relayBlocked = false; // Resetar o bloqueio manualmente
    consecutiveLowReadings = 0; // Resetar contagem de leituras baixas
  }
}

void publishHumidity(int humidityValue) {
  if (mqttClient.connected()) {
    mqttClient.publish(HUMIDITY_TOPIC, String(humidityValue).c_str());
    Serial.println("Umidade enviada com sucesso");
  } else {
    Serial.println("Erro: Não foi possível enviar a umidade, conexão MQTT não estabelecida");
  }
}

void publishRelayStatus(const char* status) {
  mqttClient.publish(RELAY_STATUS_TOPIC, status);
  Serial.print("Relé ");
  Serial.println(status);
}

void publishReservoirStatus(bool blocked) {
  mqttClient.publish(RESERVOIR_STATUS_TOPIC, blocked ? "Vazio" : "Normal");
}

void updateRelayState(int humidityValue) {
  if (deviceRunning) {
    if (!manualUnlocked) {
      if (humidityValue < MIN_HUMIDITY) {
        consecutiveLowReadings++;
        if (consecutiveLowReadings >= 5) { // Defina o número desejado de leituras baixas consecutivas para ativar o bloqueio
          relayBlocked = true;
          digitalWrite(RELAY_PIN, LOW);
          publishRelayStatus("bloqueado");
        }
      } else {
        consecutiveLowReadings = 0; // Resetar contagem de leituras baixas
      }

      if (humidityValue >= MAX_HUMIDITY || relayBlocked) {
        digitalWrite(RELAY_PIN, LOW);
        publishRelayStatus("inativo");
      } else {
        digitalWrite(RELAY_PIN, HIGH);
        publishRelayStatus("ativo");
      }

      if (previousRelayState != relayBlocked) {
        publishReservoirStatus(relayBlocked);
        previousRelayState = relayBlocked;
      }
    } else { // Se foi desbloqueado manualmente
      if (humidityValue < MIN_HUMIDITY) {
        consecutiveLowReadings++;
        if (consecutiveLowReadings >= 5) { // Defina o número desejado de leituras baixas consecutivas para ativar o bloqueio
          relayBlocked = true;
          digitalWrite(RELAY_PIN, LOW);
          publishRelayStatus("bloqueado");
        }
      } else {
        consecutiveLowReadings = 0; // Resetar contagem de leituras baixas
      }

      if (humidityValue >= MAX_HUMIDITY || relayBlocked) {
        digitalWrite(RELAY_PIN, LOW);
        publishRelayStatus("inativo");
      } else {
        digitalWrite(RELAY_PIN, HIGH);
        publishRelayStatus("ativo");
      }

      if (previousRelayState != relayBlocked) {
        publishReservoirStatus(relayBlocked);
        previousRelayState = relayBlocked;
      }
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Dispositivo desligado, não é possível enviar a umidade");
    publishRelayStatus("inativo");
  }
}



int readHumidity() {
  int humidityValue = analogRead(SOIL_SENSOR_PIN);
  humidityValue = map(humidityValue, 0, 4095, 0, 100);
  humidityValue = (humidityValue - 100) * -1;
  Serial.print("Umidade: ");
  Serial.print(humidityValue);
  Serial.println("%");

  if (humidityValue != MIN_HUMIDITY && humidityValue != MAX_HUMIDITY) {
    lastHumidityChangeTime = millis();
  }

  return humidityValue;
}

void sendHumidityData(int humidityValue) {
  publishHumidity(humidityValue);
  updateRelayState(humidityValue);
}

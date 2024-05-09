#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "NGNL";
const char* password = "Liso9445";

const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define SOIL_SENSOR_PIN 33
#define HUMIDITY_TOPIC "umidade/solo"
#define CONTROL_TOPIC "controle/dispositivo"
#define RELAY_STATUS_TOPIC "status/relé"
#define RESERVOIR_STATUS_TOPIC "status/reservatório"

#define RELAY_PIN 32
#define MIN_HUMIDITY 45
#define MAX_HUMIDITY 60

bool deviceRunning = true;
unsigned long relayActivationTime = 0;
bool relayBlocked = false;
bool previousRelayState = false;

// Callback para mensagens MQTT recebidas
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);
  if (String(topic) == CONTROL_TOPIC) {
    deviceRunning = (message == "ligar");
    Serial.println(deviceRunning ? "Dispositivo ligado" : "Dispositivo desligado");
  }
}

// Inicialização do Wi-Fi
void setupWiFi() {
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

// Inicialização do MQTT
void setupMQTT() {
  Serial.println("Conectando ao servidor MQTT...");
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Cliente")) {
      Serial.println("Conectado ao broker MQTT");
      mqttClient.subscribe(CONTROL_TOPIC);
    } else {
      Serial.print("Falha na conexão, código de retorno=");
      Serial.print(mqttClient.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// Publicar a umidade do solo via MQTT
void publishHumidity(int humidityValue) {
  if (mqttClient.connected()) {
    mqttClient.publish(HUMIDITY_TOPIC, String(humidityValue).c_str());
    Serial.println("Umidade enviada com sucesso");
  } else {
    Serial.println("Erro: Não foi possível enviar a umidade, conexão MQTT não estabelecida");
  }
}

// Enviar estado do relé via MQTT
void publishRelayStatus(const char* status) {
  mqttClient.publish(RELAY_STATUS_TOPIC, status);
  Serial.print("Relé ");
  Serial.println(status);
}

// Enviar estado do reservatório via MQTT
void publishReservoirStatus(bool blocked) {
  mqttClient.publish(RESERVOIR_STATUS_TOPIC, blocked ? "Vazio" : "Normal");
}

// Atualizar estado do relé com base na umidade do solo
void updateRelayState(int humidityValue) {
  if (deviceRunning) {
    if (humidityValue < MIN_HUMIDITY && !relayBlocked) {
      if (millis() - relayActivationTime >= 5000) {
        relayBlocked = true;
        digitalWrite(RELAY_PIN, LOW);
        publishRelayStatus("bloqueado");
      } else {
        digitalWrite(RELAY_PIN, HIGH);
        publishRelayStatus("ativo");
      }
    } else if (humidityValue >= MAX_HUMIDITY || relayBlocked) {
      digitalWrite(RELAY_PIN, LOW);
      publishRelayStatus("inativo");
    }

    if (previousRelayState != relayBlocked) {
      publishReservoirStatus(relayBlocked);
      previousRelayState = relayBlocked;
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Dispositivo desligado, não é possível enviar a umidade");
    publishRelayStatus("inativo");
  }
}

// Ler e enviar umidade do solo
void sendHumidity() {
  int humidityValue = analogRead(SOIL_SENSOR_PIN);
  humidityValue = map(humidityValue, 0, 4095, 0, 100);
  humidityValue = (humidityValue - 100) * -1;
  Serial.print("Umidade: ");
  Serial.print(humidityValue);
  Serial.println("%");

  publishHumidity(humidityValue);

  updateRelayState(humidityValue);
}

void setup() {
  Serial.begin(9600);
  setupWiFi();
  setupMQTT();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {
  if (!mqttClient.connected()) {
    setupMQTT();
  }
  mqttClient.loop();
  sendHumidity();
  delay(1000);
}

#include "WiFiHelper.h"
#include <WiFi.h>

const char* SSID = "NGNL";
const char* PASSWORD = "Liso9445";

void connectWiFi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(SSID, PASSWORD);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 5) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado");
    Serial.println("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar ao WiFi. Verifique as credenciais.");
  }
}

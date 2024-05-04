#include <Arduino.h> // Inclui a biblioteca padrão do Arduino

#define sensor 33 // Define o pino do sensor de umidade do solo - 33 corresponds to GPIO33

void setup() {
  Serial.begin(9600); // Inicializa a comunicação serial com a velocidade de 9600 bauds
}

void soilMoisture() {
  int value = analogRead(sensor); // Lê o valor do sensor de umidade do solo
  value = map(value, 0, 4095, 0, 100); // Mapeia o valor lido para o intervalo de 0 a 100
  value = (value - 100) * -1; // Inverte o valor para que uma leitura maior indique um solo mais úmido
  Serial.print("Umidade: "); // Imprime o valor lido do sensor de umidade do solo
  Serial.print(value); // Imprime o valor de umidade do solo
  Serial.println("%"); // Imprime o símbolo de percentagem
}

void loop() {
  soilMoisture(); // Chama a função de leitura do sensor de umidade do solo
  delay(1000); // Aguarda 1 segundo antes de fazer a próxima leitura
}

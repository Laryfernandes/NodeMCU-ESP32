// // Incluindo a biblioteca ESP32
// #include <Arduino.h>

// // Define o pino que será usado para controlar o relé
// #define PINO_RELE 32

// // Define o número de vezes que a bomba será ativada
// #define NUM_ATIVACOES 3

// // Define o tempo de duração da ativação da bomba em milissegundos
// #define DURACAO_ATIVACAO 2000 // 5 segundos

// int contadorAtivacoes = 0; // Variável para contar o número de vezes que o relé foi acionado

// void setup() {
//   // Inicializa a comunicação serial a 9600 bps
//   Serial.begin(9600);
  
//   // Inicializa o pino do relé como saída
//   pinMode(PINO_RELE, OUTPUT);

//   // Desliga o relé inicialmente
//   digitalWrite(PINO_RELE, LOW);
// }

// void loop() {
//   // Verifica se o número de ativações não excedeu o limite
//   if (contadorAtivacoes < NUM_ATIVACOES) {
//     // Liga o relé
//     digitalWrite(PINO_RELE, HIGH);
//     Serial.println("Relé ligado");
//     delay(DURACAO_ATIVACAO); // Espera o tempo de duração da ativação

//     // Desliga o relé
//     digitalWrite(PINO_RELE, LOW);
//     Serial.println("Relé desligado");
//     delay(1000); // Espera 1 segundo antes da próxima ativação

//     // Incrementa o contador de ativações
//     contadorAtivacoes++;
//   } else {
//     Serial.println("Número de ativações atingido. Encerrando...");
//     while (true) {} // Loop infinito para manter o programa parado
//   }
// }

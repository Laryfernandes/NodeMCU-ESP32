#include "LinkedList.h"

LinkedList::LinkedList() {
  head = NULL;
  size = 0;
}

LinkedList::~LinkedList() {
  limparLista(); // Chamando limparLista no destrutor
}

void LinkedList::limparLista() {
  while (head != NULL) {
    Node* temp = head;
    head = head->next;
    delete temp;
  }
  size = 0;
}

void LinkedList::adicionarUmidade(int umidade, bool& bloqueador, PubSubClient& mqttClient) {
  Node* newNode = new Node;
  newNode->umidade = umidade;
  newNode->next = head;
  head = newNode;

  // Limita a lista a 5 ciclos
  if (size >= 5) {
    // Verifica se os últimos 5 valores de umidade são iguais
    Node* temp = head;
    int umidadeAnterior = temp->umidade;
    bool estagnada = true;
    for (int i = 0; i < 4; i++) {
      temp = temp->next;
      if (temp->umidade != umidadeAnterior) {
        estagnada = false;
        break;
      }
    }
    if (estagnada) {
      Serial.println("Umidade estagnada");
      bloqueador = true; // Define o bloqueador como verdadeiro
    }

    // Remove o nó mais antigo
    temp = head;
    while (temp->next->next != NULL) {
      temp = temp->next;
    }
    delete temp->next;
    temp->next = NULL;
  } else {
    size++;
  }
}

void LinkedList::imprimirUmidadeArmazenada() {
  Node* temp = head;
  Serial.println("Lista de umidades quando o relé foi acionado:");
  while (temp != NULL) {
    Serial.print(temp->umidade);
    Serial.println("%");
    temp = temp->next;
  }
}

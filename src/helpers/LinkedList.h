#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <Arduino.h>
#include <PubSubClient.h>

// Estrutura para um nó da lista encadeada
struct Node {
  int umidade; // Umidade
  Node* next; // Ponteiro para o próximo nó
};

class LinkedList {
private:
  Node* head;
  int size;

public:
  LinkedList();
  ~LinkedList();
  void adicionarUmidade(int umidade, bool& bloqueador, PubSubClient& mqttClient); 
  void imprimirUmidadeArmazenada();
  void limparLista(); // Método para limpar a lista
};


#endif

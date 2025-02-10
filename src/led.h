#pragma once

#include <string>
#include <ncurses.h>
#include "progresso.h"
#include <queue>

class Serializacao { virtual std::queue<uint8_t> serializa() =0; };

class LED: public Medida, public Serializacao 
{
private:
   // Que caractére começa no momento.
   size_t cursor;

public:
   // Comprimento do visor de exibição.
   size_t comprimento;
   // Texto original dado; serve como identificador do tipo de dado.
   std::string texto;
   // Roleta de strings(do texto), que pode está se movendo.
   std::string roleta;

   /* Métodos construturos do tipo de dado:*/
   LED(std::string t, std::string r, size_t c, size_t p):
     cursor(p), comprimento(c), texto(t), roleta(r) 
   { std::cout << "Instância totalmente personalidada\n"; }
   LED(std::string dsc, size_t len);
   LED(std::string dsc); 
   LED();

   // Métodos do ncurses:
   std::string exibe();
   void desenha(const Posicao p);
   void desenha_colorido(const Posicao p);

   // Métodos virtuais que é preciso implementar.
   virtual size_t largura() override;

   // Métodos de serialização do tipo de dado:
   virtual std::queue<uint8_t> serializa() override;
   static LED deserializa(std::queue<uint8_t>&);
};

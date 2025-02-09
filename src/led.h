#pragma once

#include <string>
#include <ncurses.h>
#include "progresso.h"
#include <queue>

class Serializacao { virtual std::queue<uint8_t> serializa() =0; };

class LED: public Medida, public Serializacao 
{
private:
   // Texto original dado; serve como identificador do tipo de dado.
   std::string texto;
   // Roleta de strings(do texto), que pode está se movendo.
   std::string roleta;
   // Comprimento do visor de exibição.
   size_t comprimento;
   // Que caractére começa no momento.
   size_t cursor;

public:
   /* Métodos construturos do tipo de dado:*/
   LED();
   LED(std::string dsc); 
   LED(std::string dsc, size_t len);
   // Construtor de maior personalização.
   LED(std::string t, std::string r, size_t c, size_t p):
     texto(t), roleta(r), comprimento(c), cursor(p)
   { std::cout << "Instância totalmente personalidada\n"; }

   std::string exibe();
   void desenha(const Posicao p);
   void desenha_colorido(const Posicao p);

   // Encapsulamento de alguns campos...
   std::string get_texto()
      { return this->texto; }
   
   // Métodos virtuais que é preciso implementar.
   virtual size_t largura() override;
   virtual std::queue<uint8_t> serializa() override;
   static LED deserializa(std::queue<uint8_t>&);
};

#pragma once
// Bibliotecas do C++:
#include <string>
#include <iostream>
#include <vector>
#include <queue>
// Bibliotecas do Linux:
#include <ncurses.h>

class Medida { virtual size_t largura()=0; };

class Posicao {
/*   Posição com duas coordenadas para referênciar 'pixels' na tela do 
 * monitor, onde escrever, onder ir, é uma boa coordenada. */
public:
   // O 'y' é a linha na tela, e 'x' a coluna.
   uint8_t y, x;

   // Todos métodos para inicilizar tal.
   Posicao() =default;
   Posicao(uint8_t l, uint8_t c): y{l}, x{c} {}
   Posicao(const Posicao& a) 
      { this->x = a.x; this->y = a.y; }
};

class Progresso: public Medida {
public:
   // Contadores do progresso.
   size_t atual, total;
   // Comprimento da barra de progresso.
   uint8_t comprimento;

   // Construtores deste tipo de dado:
   Progresso()
      { this->total = 0; this->atual = 0; this->comprimento = 0; }
   // Construtor personalizado, passe todos valores de todos campos:
   Progresso(size_t n, size_t t, uint8_t c):
      atual(n), total(t), comprimento(c) {}
   Progresso(size_t n, size_t t);
   Progresso(size_t t);

   // Percentual do progresso.
   double percentual();
   // Verifica se o progresso chegou ao fim.
   bool finalizado();
   // Desenha na atual janela do ncurses.
   void desenha (Posicao p);
   void desenha_colorido(Posicao p);
   /* Formata o progresso numa string, então a retorna. */
   std::string to_string() const;

   /* Transforma atual estrutura em bytes, e também decodifica de uma. */
   std::queue<uint8_t> serializa();
   static Progresso deserializa(std::queue<uint8_t>&);

   // Operadores sobrecarregados:
   Progresso& operator=(const size_t x);
   Progresso& operator+=(const size_t n);
   Progresso& operator++();

   // Largura que tal pode ocupar na tela do ncurses.
   size_t largura() override;
};

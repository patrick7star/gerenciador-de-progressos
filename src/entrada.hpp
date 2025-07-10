#pragma once
// Bibliotecas externas:
#include <curses.h>
// Biblioteca padrão do C++:
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
// Bibliotecas legadas do C:
#include <cmath>

class Entrada {
   private:
      // Descrição de um progresso que está rodando no painel.
      std::string rotulo;

      // Os valores atuais e total a ser atingindo pelo progresso.
      size_t atual = 0;
      size_t total;

      // Dois tipos de crescimento da 'Entrada' pra debugging.
      void taxa_de_crescimento_a(void);
      void taxa_de_crescimento_b(void);

   public:
      Entrada(const std::string r, size_t a, size_t t);
      // Método construído para debug. Os dados são totalmente arbitrários.
      Entrada(void);

      // Percentual do atual progresso da 'entrada'.
      float percentual(void);
      // Faz o desenho da 'entrada' no 'ncurses'.
      void desenha(WINDOW* janela, int linha);
      // Converte sua formatação numa string.
      std::string to_string();

      // Encapsulamento do objeto.
      std::string& getRotulo(void);
      size_t getTotal(void);
      size_t getAtual(void);

      Entrada& operator++();
      bool operator==(Entrada& obj)
         { return obj.getRotulo() == this->rotulo; }

      // Processo de serialização do tipo.
      std::queue<uint8_t> serializa(void);
      static Entrada deserializa(std::queue<uint8_t>);
};

std::ostream& operator<<(std::ostream& Output, Entrada& Obj); 

#pragma once
// Outros módulos:
#include "entrada.hpp"
// Bibliotecas externas:
#include <curses.h>
// Biblioteca padrão do C++:
#include <vector>


class PainelDeProgresso {
 private:
   // Todos progressos capturados:
   std::vector<Entrada> lista;

   void desenha_moldura(void);
   void desenha_entradas(void);
   // Verifica se alguma 'entrada' está finalizada.
   void remove_entradas_finalizadas(void);

 public:
   PainelDeProgresso(void);
   ~PainelDeProgresso();

   void renderiza();
   bool todos_progressos_finalizados(void);
   constexpr std::vector<Entrada>& interno(void) 
      { return this->lista; }
};


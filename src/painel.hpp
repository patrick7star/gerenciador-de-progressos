#pragma once
// Outros módulos:
#include "entrada.hpp"
// Bibliotecas externas:
#include <curses.h>
// Biblioteca padrão do C++:
#include <vector>


class PainelDeProgresso {
 using Clock = std::chrono::system_clock;
 using TimePoint = Clock::time_point;

 private:
   // Todos progressos capturados:
   std::vector<Entrada> lista;
   TimePoint comeco;

   void desenha_moldura(void);
   void desenha_entradas(void);
   void desenha_status(void);

 public:
   PainelDeProgresso(void);
   ~PainelDeProgresso();

   void renderiza();
   bool todos_progressos_finalizados(void);
   bool permissao_pra_rodar(void);

   // Retorna a referência de 'Entradas' capturadas que foram enviadas.
   constexpr std::vector<Entrada>& interno(void) 
      { return this->lista; }
};


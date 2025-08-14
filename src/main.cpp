/*   Estou reescrevendo o 'painel de progressos'. O outro ficava muito 
 * complexo, sem nunca ter uma versão final. Por isso, ao invés de tentar 
 * implementar algo já bastante robusto para futuras modificações em 
 * primeiro, vou tentar implementar algo funcional, para depois ir adicioando
 * novas features, e tentando manter a compatibilidade.
 */

// Biblioteca padrão em C++:
#include <array>
// Antiga biblioteca do C:
#include <cassert>
// Bibliotecas externas:
// GNU library of POSIX:
// Outros módulos:
#include "entrada.hpp"
#include "painel.hpp"

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                         Construção do Entrada
 * == == == == == == == == == == == == == == == == == == == == == == == == */


#if defined(__unit_tests__) && defined(__linux__)
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                         Testes Unitários 
 * == == == == == == == == == == == == == == == == == == == == == == == == */
#include <array>
#include <thread>
#include <iostream>
#include "comunicacao.hpp"

constexpr int QTD = 8;
using namespace std;


void versao_debug(void) 
{
   PainelDeProgresso painel;
   char tecla = 0x00;
   auto& listref = painel.interno();
   constexpr auto FRAME_RATE = 400;
   Cliente caixa(listref);

   // Loop continuo de renderização.
   do {
      tecla = getch();

      if (tecla == 's') break;

      caixa.receber();
      painel.renderiza();
      napms(FRAME_RATE);

   } while(painel.permissao_pra_rodar());
}



int main(int qtd, char* args[], char* vars[]) 
{
   versao_debug();
}
#endif

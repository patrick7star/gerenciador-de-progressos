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

using std::chrono::steady_clock;
using time_point = std::chrono::time_point<steady_clock>;
using namespace std;


void versao_debug(void) 
{
   PainelDeProgresso painel;
   char tecla = 0x00;
   auto& listref = painel.interno();
   Cliente caixa(listref);
   // Ferramentas pra medida:
   auto relogio = steady_clock::now();
   constexpr auto LIMITE = 122s;
   constexpr auto FRAME_RATE = 400;

   // Loop continuo de renderização.
   do {
      tecla = getch();

      caixa.receber();
      painel.renderiza();
      napms(FRAME_RATE);

   } while(tecla != 's' && (steady_clock::now() - relogio) < LIMITE);
}

void captura_externa(void)
{
   vector<Entrada> list;
   Cliente caixa(list);

   for (int n = 1; n <= QTD; n++) {
      caixa.receber();
      this_thread::sleep_for(300ms);
   }

   auto qtd = list.size();
   cout << "Capturas feitas: " << qtd << endl;
}


int main(int qtd, char* args[], char* vars[]) 
{
   versao_debug();
   // captura_externa();
}
#endif

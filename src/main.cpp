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

constexpr int QTD = 8;

using namespace std;

void versao_debug(void) {
   const char* label = "item formado aleatóriamente";
   array<Entrada, QTD> In;
   PainelDeProgresso painel;
   char tecla = '\0';

   for (auto& entry: In)
      painel.insere(entry);

   // Loop continuo de renderização.
   do {
      tecla = getch();

      painel.renderiza();
      napms(400);
      painel.incrementa();
   
   } while(tecla != 's' && !painel.todos_progressos_finalizados());
}

void incremento_arbitrario_da_entrada(void) {
   Entrada input;

   for (int n = 1; n <= 10; n++) {
      cout << input << endl;
      ++input;
   }
}

int main(int qtd, char* args[], char* vars[]) 
{
   versao_debug();
   // incremento_arbitrario_da_entrada();
}
#endif

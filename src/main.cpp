
/*   Neste momento estou pensando numa janela ncurses, carregando múltiplas 
 * barras de progressos ao mesmo tempo. Como nomes na esquerda sobre o que
 * elas represetam, e na direita seu progresso percentual gráfico, e o 
 * margem numérica também. 
 *
 *   Isso tem como objetivo como um monitor que visualiza multiplos
 * carregamentos em plano de fundo, de um programa, ou de vários de uma vez
 * só. Já que a lib deste, pode ser copiado para vários programas, como uma
 * lib externa, ou pode, via seu binário, mostrar vários progressos, de 
 * vários programas diferentes, claro, se eles estiverem rodando, e acessar
 * corretamente os protocolos que permitem isso.
 */

#include <ncurses.h>
#include <string>
#include <iostream>
#include <vector>
// Códigos de outros módulos:
#include "progresso.hpp"

using namespace std;

class Monitor {
private:
   uint32_t quantia;
   vector<Progresso> lista;
   WINDOW* janela;
};


int main() {
   teste_do_progresso();
}

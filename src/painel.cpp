#include "painel.hpp"
#include <iostream>

using namespace std;

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                Construtores e Desconstrutor do Painel
 * == == == == == == == == == == == == == == == == == == == == == == == == */
static void configura_janela(void) {
   start_color();
   keypad(stdscr, true);
   nonl();
   noecho();
   use_default_colors();
   nodelay(stdscr, true);
}

static void inicia_paleta_de_cores(void) {
   init_pair(100, COLOR_BLACK,   -1);
   init_pair( 99, COLOR_RED,     -1);
   init_pair( 98, COLOR_GREEN,   -1);
   init_pair( 97, COLOR_YELLOW,  -1);
   init_pair( 96, COLOR_BLUE,    -1);
   init_pair( 95, COLOR_MAGENTA, -1);
   init_pair( 94, COLOR_CYAN,    -1);
   init_pair( 93, COLOR_WHITE,   -1);
}

PainelDeProgresso::PainelDeProgresso(void) {
/* Cria um 'painel', então configura várias propriedades referentes a
 * ela. Inclusive termina por realizar a primeira construção, então uma
 * renderização dela. */
   initscr();
   configura_janela();
   inicia_paleta_de_cores();

   this->renderiza();
}

PainelDeProgresso::~PainelDeProgresso(void)
   { endwin(); cout << "Finalizada interface semi-gráfica.\n"; }
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Construção e Renderização
 * == == == == == == == == == == == == == == == == == == == == == == == == */
void PainelDeProgresso::desenha_moldura(void) {
/* Desenha a moldura das janela inicialmente criada. Suas cores, bordas, e
 * os separadores dividindo as seguintes partes. */
   int largura = getmaxx(stdscr);
   string TITULO = "Painel de Progressos";
   int recuo = TITULO.length();

   TITULO += ' ';
   TITULO.insert(0, " ");

   // Moldura da janela.
   box(stdscr, ACS_VLINE, ACS_HLINE);
   // Desenhando uma célula de título ...
   move(2, 0);
   addch(ACS_LLCORNER);
   hline(ACS_HLINE, largura - 2);
   move(2, largura - 1);
   addch(ACS_LRCORNER);
   // Escrevendo o título em sí ...
   move(1, (largura - recuo) / 2);
   addstr(TITULO.c_str());
}

void PainelDeProgresso::desenha_entradas(void) {
// Desenha especificamente as 'entradas' contidas internamente no 'painel'.
   size_t linha = 3;
   WINDOW* janela = stdscr;

   for (auto& item: this->lista) 
      item.desenha(janela, linha++);
}

void PainelDeProgresso::renderiza(void) {
/* Apaga tudo que foi renderizado anteriormente, desenha as bordas e o 
 * cabeçalho, assim como a barra status; as devidas entradas, então 
 * renderiza todas estas construções. */
   erase();
   this->desenha_moldura(); 
   this->desenha_entradas();
   refresh();
}

#ifdef __unit_tests__
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Funções e Métodos pra Debuggin'
 * == == == == == == == == == == == == == == == == == == == == == == == == */
#endif

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                            Métodos privados
 * == == == == == == == == == == == == == == == == == == == == == == == == */
void PainelDeProgresso::remove_entradas_finalizadas(void) {
   int qtd = this->lista.size();
   vector<Entrada> nao_finalizados(qtd);

   for (Entrada& e: this->lista) {
      if (e.percentual() < 1.0)
         nao_finalizados.push_back(e);
   }
   this->lista = nao_finalizados;
}

bool PainelDeProgresso::todos_progressos_finalizados(void) {
/* Diz se todos os 'processos' estão finalizados, se algum não estiver, 
 * nega a proposição que vem no nome da função. Basicamente é assim que é 
 * feito algoritmo. */
   for (Entrada& e: (*this).lista) {
      if (e.percentual() < 1.0)
         return false;
   }
   return true;
}

#include "monitor.h"
// Outros módulos do projeto:
#include "auxiliar.h"
// Bibliotecas padrões do C++:
#include <algorithm>
#include <numeric>
#include <ncurses.h>
#include <cassert>
// Bibliotecas do Unix:
#include <unistd.h>

// Puxando para fora do namespace tal funções ou objetos:
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::array;

/* === === ===  === === === === === === === === === === === === === === ===
 *                   Métodos para o Enumerador
 *                         Ordenação
 * === === ===  === === === === === === === === === === === === === === = */
std::ostream& operator<<(std::ostream& saida, Ordenacao enumerador) {
   switch (enumerador) {
      case Ordenacao::Criacao:
         // saida << "Criação";
         saida << "Criacao";
         break;
      case Ordenacao::TaxaDeCrescimento:
         saida << "Taxa de Crescimento";
         break;
      case Ordenacao::Duracao:
         saida << "Mais Tempo Executando";
         break;
      case Ordenacao::ProximoDoFim:
         saida << "Quase Finalizando";
         break;
   }
   return saida;
}

std::string& operator<<(std::string& saida, Ordenacao enumerador) {
   switch (enumerador) {
      case Ordenacao::Criacao:
         // saida += "Criação";
         saida += "Criacao";
         break;
      case Ordenacao::TaxaDeCrescimento:
         saida += "Taxa de Crescimento";
         break;
      case Ordenacao::Duracao:
         saida += "Mais Tempo Executando";
         break;
      case Ordenacao::ProximoDoFim:
         saida += "Quase Finalizando";
         break;
   }
   return saida;
}

/* === === ===  === === === === === === === === === === === === === === ===
 *                   Implementação dos Métodos do
 *                         Tipo de Dado Tela
 * === === ===  === === === === === === === === === === === === === === = */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tuple>

using monitor::path;
using std::tuple;

static void define_todas_paletas_de_cores(void) 
{
   const int16_t TRANSPARENTE = -1;
   init_pair(Amarelo, COLOR_YELLOW, TRANSPARENTE);
   init_pair(Verde, COLOR_GREEN, TRANSPARENTE);
   init_pair(Vermelho, COLOR_RED, TRANSPARENTE);
   init_pair(Azul, COLOR_BLUE, TRANSPARENTE);
   init_pair(Violeta, COLOR_MAGENTA, TRANSPARENTE);
   init_pair(Branco, COLOR_WHITE, TRANSPARENTE);
   init_pair(AzulMarinho, COLOR_CYAN, TRANSPARENTE);
}

static void configura_janela_iniciada(WINDOW* janela) {
   #ifdef __debug__
   cout << "É possível mudar alguma cor?";
   if (can_change_color())
      cout << "Sim\n" << endl;
   else
      cout << "Não" << endl;
   #endif

   // Ativa cores e permite a transparência.
   start_color();
   use_default_colors();
   // Omite o cursor. Também, acaba com o piscar de teclas pressionadas.
   curs_set(0);
   noecho();
   // A fonte será sempre em "negrito".
   attrset(A_BOLD);
   // Aceitas teclas especiais, também não bloquea precionamento de teclas.
   keypad(janela, true);
   nodelay(janela, true);
}

static tuple<path, int> criacao_do_canal_de_insercao(void) {
   const char* const NOME_NP = "inserção";
   int file_descriptor;

   if (mkfifo(NOME_NP, 0664) == 0) {
      cout << "Canal de inserção criado com sucesso.\n";
      file_descriptor = open(NOME_NP, O_RDONLY | O_NONBLOCK);

      if (file_descriptor == -1) {
         switch(errno) {
         case EEXIST:
            cout << "O canal já existe!\n";
            break;
         default:
            cout << "Outro erro ainda não trabalhado!\n";
         }
      }
   } else 
      cout << "Canal de inserção não criado!\n";
   return std::make_tuple(path(NOME_NP), file_descriptor);
}

monitor::Tela::Tela() 
{
   cout << "Decide a taxa de crescimento como a ordem padrão.\n";
   this->lista = vector<Entrada>();
   this->ordem = Ordenacao::TaxaDeCrescimento;

   /* Agora, tanto a criação como configuração, referente a janela(tela)
    * do ncurses -- a parte gráfica. */
   this->janela = initscr();

   // Configuração geral da janela trabalhada.
   configura_janela_iniciada(this->janela);
   // Configurando as cores do programa...
   define_todas_paletas_de_cores();

   /* Cria também o 'named pipe' que aceita 'Entradas' forasteiras. Registra
    * o caminho, e o abre também. */
   auto tupla = criacao_do_canal_de_insercao();
   std::tie(this->canal_de_insercao, this->fd) = tupla;
}

monitor::Tela::~Tela() {
   // Então remove o canal de comunicação.
   const char* pathname = this->canal_de_insercao.c_str();

   // Finaliza a janela do ncurses.
   endwin();
   cout << "A tela foi finalizada com sucesso.\n";
   // Finaliza o namedpipe ...
   if (close(this->fd) == 0)
      cout << "O named pipe foi fechado com sucesso.\n";

   if (unlink(pathname) == -1) {
      cout << "\nAlgum erro ocorreu na eliminação de " << pathname << endl;
      cout << this->canal_de_insercao << std::endl;

      switch (errno) {
      case EACCES:
         cout << "Sem acesso de permissão de escrita.\n";
         break;
      case EBUSY:
         cout << "O pipe está sendo usado.\n";
         break;
      case ENOENT:
         cout << "Tal arquivo não existe!\n";
         break;
      case EPERM:
         cout << "É um diretório.\n";
         break;
      case EROFS:
         cout << "Diretório do arquivo está no modo read-only.\n";
         break;
      }
   }
}

string percentual_medio(vector<monitor::Entrada>& l)
{
   string fmt{"percentual medio: "};
   vector<double> percentuais;

   // Computando a média dos percentuais...
   [[maybe_unused]]
   auto _saida = std::transform(
      l.begin(), l.end(), 
      back_inserter(percentuais),
      [](monitor::Entrada& e) 
         { return e.bar.percentual(); }
   );
   auto pa = percentuais.begin();
   auto pb = percentuais.end();
   double S = std::accumulate(pa, pb, 0.0);
   auto t = (double)(l.size());
   double mA = S / t;

   // Formando string do percentual...
   fmt.append(std::to_string(mA));
   for (int i = 1; i <= 5; i++)
      fmt.pop_back();
   fmt.push_back('%');

   return fmt;
}

void monitor::Tela::desenha_status()
{
/* Desenha barra de status na parte inferior da tela, onde ficam vários 
 * dados agregrados dos progressos em execução. */
   vector<double> percentuais;
   const int MARGEM_STATUS = 1;
   const char BRANCO = ' ';

   size_t executando = std::count_if(
      this->lista.begin(), this->lista.end(),
      [](Entrada& e) { return (not e.bar.finalizado()); }
   );

   string fmt{"execucao: "};
   int Y = getmaxy(this->janela) - 1;
   int X = getmaxx(this->janela);
   const char* sep = " | ";

   fmt += std::to_string(executando);
   fmt.append(sep);
   fmt += percentual_medio(this->lista);
   fmt.append(sep);
   fmt.append("ordenacao: \'");
   fmt << this->ordem;
   fmt.push_back('\'');
   // Preenchendo resta da barra apenas com vázio, para acompanhar a 
   // formatação em destaque.
   int restante = X -  MARGEM_STATUS - fmt.length();
   for (int i = 1; i <= restante; i++)
      fmt.push_back(BRANCO);

   attron(A_STANDOUT);
   mvaddstr(Y, MARGEM_STATUS, fmt.c_str());
   attroff(A_STANDOUT);
}

void monitor::Tela::renderiza() {
   // Constrói as "imagens" dos objetos na tela.
   int linha = 2;
   [[maybe_unused]]
   size_t total = this->lista.size();

   erase();
   for (auto& entry: this->lista)
   {
      int L = getmaxx(this->janela);

      Posicao ponto_r(linha, 1);
      Posicao ponto_b(linha++, L - entry.bar.largura());

      // entry.rotulo.desenha(ponto_r);
      // entry.bar.desenha(ponto_b);
      entry.rotulo.desenha_colorido(ponto_r);
      entry.bar.desenha_colorido(ponto_b);
   }
   this->desenha_status();

   refresh();
   napms(this->VELOCIDADE);
}

monitor::Entrada& monitor::Tela::operator[](string nome) 
{
/* Não existe nenhuma hashmap dentro da estrutura, apenas uma lista, loga
 * tal busca sempre dará algo em tempo linear O(n). */
   for (Entrada& entry: this->lista) 
   {
      string corresponde = entry.rotulo.texto;

      if (corresponde == nome)
         return entry;
   }

   string msg_erro = "não existe tal progresso aqui.";
   throw std::invalid_argument(msg_erro);
}

/* Ordena a lista de entradas internas. */
void monitor::Tela::ordena() {}

void monitor::Tela::adiciona(Entrada&& obj) {
   this->lista.push_back(obj); 
   this->ordena();
}

array<int, 2> monitor::Tela::dimensao() {
   array<int, 2> par_yx;

   par_yx[0] = getmaxy(this->janela);
   par_yx[1] = getmaxx(this->janela);

   return par_yx;
}

bool monitor::Tela::tudo_finalizado()
{
/* Retorna se todas entradas contidas na listas estão finalizadas. Um
 * método bem útil para fazer um loop continuo, que apenas finaliza quando
 * todas entradas da listas tenham esgotado seus progressos. */
   return std::all_of(
      this->lista.begin(), 
      this->lista.end(), 
      [](Entrada& e)
         { return e.bar.finalizado(); }
   );
}

void monitor::Tela::altera_ordenacao() {
   const int TOTAL = sizeof(Ordenacao);
   const int p = (int)this->ordem;

   this->ordem = (Ordenacao)((p + 1) % TOTAL);
}
#ifdef __unit_tests__
/* === === ===  === === === === === === === === === === === === === === ===
 *                      Testes Unitários
 * === === ===  === === === === === === === === === === === === === === = */
#include <cstring>
#include <cstring>
#include <chrono>
#include <ctime>
#include <thread>
#include <new>

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::chrono::seconds;
using entrada::EntradaPipe;
using auxiliar::queue;

void experimento_varias_features_do_curses(void)
{
   WINDOW* janela = initscr();

   start_color();
   use_default_colors();
   attrset(A_BOLD);

   const int16_t TRANSPARENTE = -1;
   init_pair(99, COLOR_YELLOW, TRANSPARENTE);
   init_pair(98, COLOR_GREEN, TRANSPARENTE);
   init_pair(97, COLOR_RED, TRANSPARENTE);
   init_pair(96, COLOR_BLUE, TRANSPARENTE);
   init_pair(95, COLOR_MAGENTA, TRANSPARENTE);
   init_pair(94, COLOR_WHITE, TRANSPARENTE);

   for (int i = 0; i < 6; i++) {
      color_set(99 - i, NULL);
      int y = 2*i + 1, x = 5;
      mvaddstr(y, x, "Isso é uma frasem em ...");
   }

   const char* msg = "Este experimento chegou ao final.";
   int y = getmaxy(janela) - 3;
   attron(A_UNDERLINE | A_DIM);
   mvaddstr(y, 1, msg);
   attroff(A_UNDERLINE | A_DIM);

   refresh();
   napms(3200);
   endwin();
}

void atuais_cores_do_curses(void) {
   cout << "Tetanto estimar quantia: " << sizeof(Ordenacao)<< endl;

   cout << "Total de cores: " << COLORS << endl
      << "Branco: " << COLOR_WHITE << endl
      << "Azul: " << COLOR_BLUE << endl
      << "Amarelo: " << COLOR_YELLOW << endl
      << "Verde: " << COLOR_GREEN << endl
      << "Violeta: " << COLOR_MAGENTA << endl
      << "Azul Marinho: " << COLOR_CYAN << endl
      << "Vermelho: " << COLOR_RED << endl;
}

void visualizacao_do_enum_ordenacao(void)
{
   const char* S = ": ";

   cout << Ordenacao::TaxaDeCrescimento << S << (int)Ordenacao::TaxaDeCrescimento
      << endl << Ordenacao::Duracao << S << (int)Ordenacao::Duracao
      << endl << Ordenacao::ProximoDoFim << S << (int)Ordenacao::ProximoDoFim
      << endl << Ordenacao::Criacao << S << (int)Ordenacao::Criacao << endl;
}

// ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~
static uint8_t* queue_to_array(queue<uint8_t>&& q) {
   uint8_t* seq = new uint8_t[q.size()];
   size_t cursor = 0;

   while (!q.empty()) { seq[cursor++] = q.front(); q.pop(); }
   return seq;
}

template <typename T>
void drop(T&& self) 
   { /* Será automáticamente liberado aqui. */ }

void progresso_paralelo(string rotulo, size_t fim) {
   auto PAUSA = milliseconds(800);
   auto X = EntradaPipe(path("via"), rotulo, fim); 

   do {
      X += 10;
      X.atualiza_externo();
      std::this_thread::sleep_for(PAUSA);

   } while(X.bar.percentual() < 1.00);

   drop(std::move(X));
   std::exit(0);
}

void choca_varias_entradas_paralelas(void) {
   if (fork() == 0)
      progresso_paralelo(string("pacote azul"), 300);
   else {
      if (fork() == 0)
         progresso_paralelo(string{"pacote vermelho"}, 200);
      else {
         if (fork() == 0)
            progresso_paralelo(string{"pacote amarelo"}, 400);
      }
   }
}

// ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~
void construindo_por_tentativa_e_erro_tela(void)
{
   using namespace monitor;
   using namespace std::chrono;

   Tela screen;

   const int T = 6;
   size_t totais[T] = { 30, 500, 170, 100, 200, 300 };
   size_t taxas[T] = {2, 18, 8, 8, 10, 9};
   // array<Entrada, T> entries;
   const string legendas[] = {
      "imagem_de_um_pinguim_jantando_com_um_leão.jpg",
      "volume da caixa de água do prédio(em L)",
      "trajetoria até casa(em km)",
      "período de chuva (em mL)",
      "duração do dia(em h)",
      "Entregas dos estoques de 10 anos atrás via cargueiro"
   };
   // Personalização:
   const uint8_t cV = 38;
   const uint8_t cB = 30; 
   auto agora = system_clock::now();

   choca_varias_entradas_paralelas();

   for (int i = 0; i < T; i++) {
      Entrada e(legendas[i], 0, totais[i], cV, cB, 0.0, agora);
      screen.adiciona(Entrada(e));
   }

   // Tecla de escape do teclado, tirada da tabela ASCII.
   const int KEY_ESC = 0x1b;

   do {
      // Aumentas respectivas entradas...
      for (int i = 0; i < T; i++)
         screen[legendas[i]] += taxas[i];
      screen.renderiza();

      int tecla = getch();
      // Apertar <backspace> ou <escape> para abandonar o programa.
      if (tecla == KEY_BACKSPACE || tecla == KEY_ESC)
         break;
      else if (tecla == 'i')
         screen.altera_ordenacao();

   } while(!screen.tudo_finalizado());
} 


int main(void) {
   // atuais_cores_do_curses();
   // visualizacao_do_enum_ordenacao();
   construindo_por_tentativa_e_erro_tela();
}
#endif

#include "monitor.h"
#include "auxiliar.h"
#include <algorithm>
#include <numeric>
#include <ncurses.h>
#include <cassert>

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::seconds;
using std::array;

/* Larguras globais dos elementos redimensionados da entrada. A padronização
 * é uma boa prática para algo bem organizado. */
uint8_t VISOR_LARGURA = 32;
uint8_t BARRA_LARGURA = 41;

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
 *                   Implementação dos Métodos do Tipo de Dado
 *          Entrada, sejam eles operadores, constrututores, e etc...
 * === === ===  === === === === === === === === === === === === === === = */
monitor::Entrada::Entrada(string label, size_t a, size_t T, uint8_t vC, 
  uint8_t bC, double tI, time_point<Clock> tC)
{
/* Construtor que pega todos parâmetros necessários.*/
   this->rotulo = LED(label, vC);
   this->bar = Progresso(a, T, bC);
   this->criacao = tC;
   this->taxa = tI;
}

monitor::Entrada::Entrada(string label, size_t t) 
{
   this->rotulo = LED(label, VISOR_LARGURA);
   this->bar = Progresso(0, t, BARRA_LARGURA);
   this->criacao = system_clock::now();
   this->taxa = 0.0;
}

monitor::Entrada::Entrada() {
   this->rotulo = LED("entrada temporário", VISOR_LARGURA);
   this->bar = Progresso(0, 1, BARRA_LARGURA);
   this->criacao = system_clock::now();
   this->taxa = 0.0;
}
 /* --- --- --- -- -- Sobrecarga de Operadores da Entrada --- --- --- --- */
bool monitor::Entrada::operator<(Entrada& obj) {
   // Medida da instância, e então medida do argumento, respectivamente.
   auto a = this->bar.percentual();
   auto b = obj.bar.percentual();

   return a < b;
}

bool monitor::Entrada::operator>(Entrada& obj) {
   // Medida da instância, e então medida do argumento, respectivamente.
   auto a = this->bar.percentual();
   auto b = obj.bar.percentual();

   return a > b;
}

// monitor::Entrada& monitor::Entrada::operator+=(size_t quantia) 
auto monitor::Entrada::operator+=(size_t quantia) -> Entrada&
   { this->bar += quantia; return *this; }

auto monitor::Entrada::operator++() -> Entrada& 
   { this->bar += 1; return *this; }

auto monitor::Entrada::serializa() -> queue<uint8_t> {
/* A conversão de dados se dará na seguinte forma: Primeiro o texto, rótulo,
 * nome,... como você quiser chamar-lo, entretanto, antes de injetar isso
 * como bytes, é preciso armazenar de quantos bytes estamos realmente
 * falando. O próximo é os dois valores que definem o progresso, o atual
 * valor, e a meta, ambos que são inteiros positivos com tamanho da máquina.
 * A taxa do aumento percentual também é importante, não a mais, poderia
 * sim, ser computada neste atual processo, porém se já vem processado, 
 * porque fazer isso. O último ponto traduzido em bytes é o 'timestamp' que
 * criou o "progresso", ou seja, quanto tempo está executando isso, ele é
 * um inteiro com sinal, mais tamanho de máquina.
 *
 *   Seguir tal ordem é importante para futuramente a deserialização e 
 * decodificação de tal continuos array de bytes seja facilitada, ela 
 * seguirá a mesma sequência de processamento desta função aqui. */
   using namespace auxiliar;

   const int sz = sizeof(size_t);
   queue<uint8_t> Out;
   /* Selo de tempo quando criado tal coisa. */
   time_t t = system_clock::to_time_t(this->criacao);
   
   auto Out_a = this->rotulo.serializa();
   auto Out_b = serializa_inteiro((size_t)t); 
   auto Out_c = serializa_decimal(this->taxa);
   auto Out_d = this->bar.serializa();
   /* Contabilizando o total de bytes do conjunto acima, inclusive deste
    * aqui. A letra bem distante das demais indica que não seguirá a ordem
    * de concatenação das demais. Ele no quesito tem que ser o primeiro
    * a ser concatenado, pois indica o total de bytes a extrair de uma
    * fila contendo a aglomeração de todos bytes. */
   auto Out_z = serializa_inteiro(
      Out_a.size() + Out_b.size() + 
      Out_c.size() + Out_d.size() + sz
   );

   anexa_fila(std::move(Out_z), Out);
   anexa_fila(std::move(Out_a), Out);
   anexa_fila(std::move(Out_b), Out);
   anexa_fila(std::move(Out_c), Out);
   anexa_fila(std::move(Out_d), Out);

   /* Convertendo tanto o valor 'atual' do progresso, como seu valor 
    * 'total', ambos inteiros positivos da máquina. */
   return Out;
}

auto monitor::Entrada::deserializa(queue<uint8_t>& In) -> Entrada {
/* O esquema é seguir o procedimento acima. Como o resultado é uma fila 
 * de bytes, então os primeiros serializados, serão obviamente os primeiros
 * a serem deserializado. O primeiro aqui é o tanto de bytes que a Entrada
 * ocupa, por ser uma geração variavel foi necessário colocar tal tanto
 * de bytes codificados no começo da "linguiça de bytes". Decodificado eles
 * você pode extrair o total de bytes necessários para decodificar tal 
 * tipo. */
   using namespace auxiliar;

   auto bytes = extrai_n_bytes(In, sizeof(size_t));
   auto total = deserializa_inteiro(bytes);

   assert (In.size() >= total);
   // Restantes do bytes a extrair, já faz tudo de uma vez só.
   auto r = extrai_n_bytes(bytes, total);

   auto rotulo  = LED::deserializa(r);
   auto criacao = deserializa_inteiro(r);
   auto taxa    = deserializa_decimal(r);
   auto bar     = Progresso::deserializa(r);

   // Tudo deve ter sido devidamente extraído.
   assert (r.empty());

   return Entrada();
}



/* === === ===  === === === === === === === === === === === === === === ===
 *                   Implementação dos Métodos do
 *                         Tipo de Dado Tela
 * === === ===  === === === === === === === === === === === === === === = */
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

monitor::Tela::Tela() 
{
   cout << "Decide a taxa de crescimento como a ordem padrão.\n";
   this->lista = vector<Entrada>();
   this->ordem = Ordenacao::TaxaDeCrescimento;

   #ifdef __debug__
   cout << "É possível mudar alguma cor?";
   if (can_change_color())
      cout << "Sim\n" << endl;
   else
      cout << "Não" << endl;
   #endif

   /* Agora, tanto a criação como configuração, referente a janela(tela)
    * do ncurses -- a parte gráfica. */
   this->janela = initscr();

   // Configuração geral da janela trabalhada.
   start_color();
   use_default_colors();
   curs_set(0);
   noecho();
   attrset(A_BOLD);
   keypad(this->janela, true);
   nodelay(this->janela, true);

   // Configurando as cores do programa...
   define_todas_paletas_de_cores();
}

monitor::Tela::~Tela() {
   endwin();
   cout << "A tela foi finalizada com sucesso.\n";
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
      string corresponde = entry.rotulo.get_texto();

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

#ifdef __UT_MONITOR__
/* === === ===  === === === === === === === === === === === === === === ===
 *                      Testes Unitários
 * === === ===  === === === === === === === === === === === === === === = */
#include <cstring>
#include <chrono>

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

void construindo_por_tentativa_e_erro_tela(void)
{
   using namespace monitor;
   using namespace std::chrono;

   Tela screen;

   const int T = 6;
   size_t totais[T] = { 30, 500, 170, 100, 200, 300 };
   size_t taxas[T] = {1, 9, 4, 4, 5, 2};
   array<Entrada, T> entries;
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
   atuais_cores_do_curses();
   visualizacao_do_enum_ordenacao();
   construindo_por_tentativa_e_erro_tela();
}
#endif

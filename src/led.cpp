// Bibliotecas padrão do C++:
#include <random>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>
// Bibliotecas do próprio módulo:
#include "monitor.h"
#include "led.h"
// Bibliotecas externas:
extern "C" {
   #include "conversao.h"

   struct Bytes string_to_bytes(char*);
   char* from_bytes_to_string(struct Bytes*);
   void sizet_to_bytes (size_t, uint8_t*);
};

// Encurtando nomes dos objetos...
using std::string;
using std::cout;
using std::endl;
using std::uniform_int_distribution;
using std::random_device;
using std::vector;
using std::queue;

string string_aleatoria(int n)
{
   random_device motor;
   uniform_int_distribution<uint8_t> gerador(65, 126);
   string result;

   for (int i = 1; i <= n; i++) {
      auto code = gerador(motor);
      result.push_back((char)code);
   }
   return result;
}

string concatena_em_si_propria(string& padrao, int n)
{
   string fmt = padrao;

   // Concatenando várias vezes a string passada nela mesma.
   for (int i = 1; i <= n; i++) {
      fmt.append(padrao);
      fmt.append(" -- ");
   }
   return fmt;
}

// Padronização da quantidade de vezes que uma string será concatenada
// nela mesma.
const int N = 30;
// Comprimento da string aleatória gerada.
const int M = 40;

LED::LED()
{
   this->texto = string_aleatoria(M);
   this->roleta = concatena_em_si_propria(this->texto, N);

   /* Computando o comprimento do visor do LED... */
   float P = 0.43;
   size_t t = texto.length();
   this->comprimento = (size_t)((float)t * P);
   this->cursor = 0;
}

LED::LED(string dsc): texto(dsc) 
{
   string padrao = dsc;
   float P = 0.50;
   size_t t = texto.length();

   this->texto = dsc;
   this->roleta = concatena_em_si_propria(dsc, N);
   // Coprimento do visor fica metade da string.
   this->comprimento = (size_t)((float)t * P);
   // Sempre começar na posição zero.
   this->cursor = 0;
}

LED::LED(string dsc, size_t len) {
   this->texto = dsc;
   this->roleta = concatena_em_si_propria(dsc, N);
   // Coprimento do visor fica metade da string.
   this->comprimento = len;
   // Sempre começar na posição zero.
   this->cursor = 0;
}


string LED::exibe()
{
/*   Retorna a string, baseado no comprimento do LED, ele retorna a volta
 * se o comprimento passar o atual comprimento da string. 
 *   Toda vez que tal método é chamada, a "roleta" da string gira, tenha
 * precaução em que período de tempo chama-se este método. Um segundo é
 * uma boa taxa, mais é toleravel algo como acima de 300 milisésimos. */
   auto C = this->comprimento;
   auto posicao = this->cursor + C;
   string visor;

   /* Se atravessar barreira da string, apenas reseta para o ínicio. 
    * Inclusive o cursor que marca onde está a visualização agora. */
   if (posicao >= this->roleta.length()) 
      this->cursor = 0;

   // Dar sinais de não rotação deste LED!
   if (this->texto.length() >= this->comprimento) {
      // Retira determinado trecho da roleta.
      auto a = this->cursor;
      visor = this->roleta.substr(a, C);
      // Move ela uma casa a frente.
      this->cursor += 1;
      // Dá sensação de continuidade ao LED.
      visor.append("...");
   } else
      visor = this->texto;
   return visor;
}

void LED::desenha(const Posicao p)
{
   string visor = this->exibe();
   const char* msg = visor.c_str();

   // Obs.: Não precisa da instância principal de janela para escrever.
   mvaddstr(p.y, p.x, msg);
}

void LED::desenha_colorido(const Posicao p)
{
   string visor = this->exibe();
   const char* msg = visor.c_str();

   // Obs.: Não precisa da instância principal de janela para escrever.
   color_set(Amarelo, NULL);
   mvaddstr(p.y, p.x, msg);
   color_set(Branco, NULL);
}

/* === === ===  === === === === === === === === === === === === === === ===
 *                     Interface Herdada à Classe
 * === === ===  === === === === === === === === === === === === === === = */
size_t LED::largura() 
   { return this->comprimento + 3; }

/* === === ===  === === === === === === === === === === === === === === ===
 *								Processo de Serialização
 *									e Deserialização
 * === === ===  === === === === === === === === === === === === === === = */
#include "auxiliar.h"

using namespace auxiliar;

queue<uint8_t> LED::serializa() 
{
/*   O que será realmente serializado de dado? A string que representa o 
 * rótulo é claro, porém o cursor onde está na roleta é mais complicado. 
 * Até poderia colocar, más em todos casos seriam descartado no caso que
 * desejo esta serialização. Talvez seja isso mesmo, serializado todos dados
 * porém na reconstrução, apenas descarto os que não julgo como importantes
 * no momento.
 *
 *   Para não haver confusão na hora de escrever o deserializador, farei
 * a serialização de acordo com ordem de definição dos campos da classe.
 *
 *   Como também este tipo de dado não tem tamanho determinado em tempo de
 * compilação, colocarei o total de bytes que toma no começo da string de
 * bytes. Fica um modo bem interresante de descartar bytes inválidos que 
 * não checam com o tamanho imediatamente. */
	auto texto        = serializa_string(this->texto);
	auto roleta       = serializa_string(this->roleta);
	auto comprimento  = serializa_inteiro(this->comprimento);
	auto cursor       = serializa_inteiro(this->cursor);
   /* Fila com os bytes iniciais de 'texto_bytes'. */
   queue<uint8_t> saida;

   anexa_fila(std::move(texto), saida);
   anexa_fila(std::move(roleta), saida);
   anexa_fila(std::move(comprimento), saida);
   anexa_fila(std::move(cursor), saida);

   return saida;
}

LED LED::deserializa(queue<uint8_t>& In)
{
/* Cada chamada de deserialização de algum tipo abaixo, deserializa uma 
 * fatia de bytes, e transforma no respectivo tipo primitivo de dado. O
 * total, retira bytes da serialização de um LED, que como já disse é uma
 * estrutura dinâmica por causa das strings.
 */
	auto texto        = deserializa_string  (In);
	auto roleta       = deserializa_string  (In);
	auto comprimento  = deserializa_inteiro (In);
	auto cursor       = deserializa_inteiro (In);

	return LED(texto, roleta, comprimento, cursor);
}


#ifdef __UT_LED__
/* === === ===  === === === === === === === === === === === === === === ===
 *                      Testes Unitários
 * === === ===  === === === === === === === === === === === === === === = */
#include <cstdarg>
extern "C" {
  #include "teste.h"

  void executa_testes_a(bool, int, ...);
}

void geracao_de_strings_randomicas(void)
{
   for (int i = 1; i <= 23; i++)
      cout << i << "ª. " << string_aleatoria(19) << endl;
}

void led_simples_exibicao(void)
{
   LED objeto("Hoje está um dia lindo, o céu está bastante azul.");

   cout << "Objeto está rotacionando ..." << endl;
   for (int i = 1; i <= 12; i++)
      cout << "\t \"" << objeto.exibe() << '\"' << endl;
}

void roletas_no_ncurses(void)
{
   auto janela = initscr();
   LED exemplos[] = {
      LED("All the most titans, show your faces", 30),
      LED("Most everyday, everything you says", 30),
      LED("They come dreaming, pass about addict, had my heart", 40),
      LED("Holding everything with you now, move up now", 30),
      LED("You, love, love, love, than i can't love you", 13),
      LED("You're welcome in, every time i see you above the face", 37),
      LED("We have love in a safety place", 10)
   };

   // configuração básica dela:
   cbreak(); noecho();

   cout << "Observe como a frase parece está rotacioando!\n";
   // Desenhando na grade de exibição ...
   uint8_t y = 1;
   do {
      // Apaga tudo...
      erase();
      // Reeescreve a borda...
      box(janela, 0, 0);

      for (int k = 1; k <= 7; k++) {
         Posicao ponto(k * 2, 3);
         exemplos[k - 1].desenha(ponto);
      }

      refresh();

      // Taxa de velocidade do frame.
      napms(800);
   } while (y++ < 10);

   refresh();
   endwin();
}

void visualiza_extrai_n_bytes(void) {
   queue<uint8_t> a;

   for (uint8_t i = 1; i <= 11; i++)
      a.push(i);

   print_queue(a);
   auto b = extrai_n_bytes(a, 6);
   
   cout << "\nApós extração:" << endl;
   print_queue(a);
   print_queue(b);
}

void converte_primeiros_bytes_mas_nao_consome_os(void) {
   string s{"três"};
   string S{"abacaxi"};

   auto a = serializa_string(s);
   auto b = serializa_string(S);

   cout << "Bytes da string com multibytes characters:\n";
   print_queue(a);
   cout << "\nBytes da 'narrow string':\n";
   print_queue(b);

   int x = espia_tamanho(a);
   int X = espia_tamanho(b);

   cout << "tamanho de \'" << s << "\': " << x << endl;
   cout << "tamanho de \'" << S << "\': " << X << endl;

   assert (s.length() == x);
   assert (S.length() == X);
}

void serializacao_da_string(void) {
   string s{"maçã"};
   string S{"nada"};

   auto m = serializa_string(s);
   auto n = serializa_string(S);

   cout << "Bytes da string com multibytes characters:\n";
   print_queue(m);
   cout << "\nBytes da 'narrow string':\n";
   print_queue(n);

   auto a = deserializa_string(m);
   cout << "Retornado com sucesso.\n";
   auto b = deserializa_string(n);

   cout << a << '\t' << b << endl;
   assert (a == "maçã");
   assert (b == "nada");
}

void serializacao_do_led(void) {
   auto amostra = LED("seven nations army by White Stripes");
   for (int i = 1; i < 9; i++)
      amostra.exibe();

	auto fila_de_bytes = amostra.serializa();
   cout << "fila_de_bytes: " << fila_de_bytes.size() << " bytes\n";


	LED::deserializa(fila_de_bytes);
}


int main(void) 
{
   executa_testes_a(
      true, 7,
      geracao_de_strings_randomicas, false,
      led_simples_exibicao, false,
      roletas_no_ncurses, false,
      visualiza_extrai_n_bytes, true,
      converte_primeiros_bytes_mas_nao_consome_os, true,
      serializacao_da_string, true,
      serializacao_do_led, true
   );
}
#endif

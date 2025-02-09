// Outros módulos deste projeto:
#include "progresso.h"
#include "monitor.h"
#include "auxiliar.h"
// Biblioteca do C:
#include <cmath>
#include <cassert>

// using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::vector;
using auxiliar::queue;

const int MARGEM = 2;

/* === === ===  === === === === === === === === === === === === === === ===
 *                      Construtores
 * === === ===  === === === === === === === === === === === === === === = */
Progresso::Progresso(size_t n, size_t t) {
/* Atual valor, e o total que precisa-se atingir. O comprimento será 
 * decidido de forma arbitrária. */
   // Começo é do zero.
   this->atual = 0; this->total = t;
   // Comprimento do progresso será selecionado arbitrariamente.
   this->comprimento = 20;
}

Progresso::Progresso(size_t t): total(t) {
   // Começo é do zero.
   this->atual = 0;
   // Comprimento do progresso será selecionado arbitrariamente.
   this->comprimento = 30;
}

/* === === ===  === === === === === === === === === === === === === === ===
 *                      Funções Auxiliares
 *                            e para Debugging
 * === === ===  === === === === === === === === === === === === === === = */
string percentual_em_str(double p)
{
   string fmt = std::to_string(p);
   // Trucando dígitos desnecessários, deixando apenas dois.
   fmt.pop_back(); fmt.pop_back();
   fmt.pop_back(); fmt.pop_back();
   fmt.pop_back();
   fmt.push_back('%');

   if (p < 10.0)
      { fmt.insert(0, 2, ' '); }
   else if (p >= 10.0 and p < 100.0)
      { fmt.insert(0, 1, ' '); }

   return fmt;
}

string rascunha_barra(double p, uint8_t C)
{
/* Forma a string que representa a barra de progresso. */
   const char VACUO = '.', PREENCHIDO = 'o';
   string resultado_fmt;
   size_t L = (size_t)ceil(C * p / 100.0);
   size_t i = 1;

   resultado_fmt.push_back('[');
   // Preechendo de acordo com o percentual.
   do {
      if (i++ <= L)
         resultado_fmt.push_back(PREENCHIDO);
      else
         resultado_fmt.push_back(VACUO);
   } while (i <= C);
   // Finalizando o progresso em sí.
   resultado_fmt.push_back(']');
   resultado_fmt.push_back(' ');

   // Anexando o percentual.
   resultado_fmt.append(percentual_em_str(p));

   return resultado_fmt;
}

string Progresso::to_string() const
{
/* É mais um método auxiliar ao operador<< do que algo que foi feito para
 * ser útil no programa final. Novamente, uma ferramenta de debug. */
   size_t t = this->total, n = this->atual;
   string fmt{"Progresso"};
   auto total_str = std::to_string(t);
   auto atual_str = std::to_string(n);

   fmt.append(" em ");
   fmt.append(atual_str);
   fmt.append(" de ");
   fmt.append(total_str);

   return fmt;
}

ostream& operator<<(ostream& saida, Progresso& obj)
{
/* Output principalmente para debug, na aplicação final tal método é bem
 * inútil, até não recomendado de usar. */
   saida << obj.to_string();
   return saida;
}
/* === === ===  === === === === === === === === === === === === === === ===
 *                   Sobrecarga de Operadores, aqueles
 *                      que Atualizam o Valor
 * === === ===  === === === === === === === === === === === === === === = */
Progresso& Progresso::operator=(const size_t x)
{
/* Modo de atualizar, aqui são só aceito valores igual ou maior do que
 * o existente. Se for dado algo abaixo, então nada será alterado. */
   size_t A = this->atual;

   if (x > A)
      this->atual = x;
   return *this;
}

Progresso& Progresso::operator++() {
/* Modo prático, e bem idiomático do C++, de atualizar tal progresso em
 * uma única unidade. */
   if (this->atual < this->total)
   // Apenas acrescenta se não atingiu o fim.
      this->atual++;

    return *this;
}

Progresso& Progresso::operator+=(const size_t n) {
   this->atual += n;

   if (this->atual > this->total)
   // Ajusta caso tenha sobreposto o total.
      this->atual = this->total;

   return *this;
}

double Progresso::percentual()
{
/* Apenas computa o percentual do progresso configurado. */
   double a = (double)this->atual;
   double T = (double)this->total;

   return 100.0 * a / T;
}

void Progresso::desenha(Posicao p)
{
/* Dado a janela que tais "progressos" estão sendo mostrados, este método
 * atualiza, com atuais valores, este objeto nela. Apenas desenha, nada
 * de atualizar a tela em sí(refresh). */
   int y = p.y, x = p.x - MARGEM;

   /* Recria barra de progresso, de acordo com o percentual do momento. */
   double phase = this->percentual();
   uint8_t len = this->comprimento;
   string barra = rascunha_barra(phase, len);

   // Finalmente desenha a barra de progresso na posição dada.
   mvaddstr(y, x, barra.c_str());
}

void Progresso::desenha_colorido(Posicao p)
{
   int y = p.y, x = p.x - MARGEM;
   /* Recria barra de progresso, de acordo com o percentual do momento. */
   double phase = this->percentual();
   uint8_t len = this->comprimento;
   string barra = rascunha_barra(phase, len);

   /* Apenas aciona, se houver possibilidade de já ter carregado no 
    * mínimo uma barrinha. */
   int a = barra.find('[') + 1;
   int b = barra.find('.');
   int c = barra.find(']') + 1;

   if (a == string::npos || b == string::npos || c == string::npos)
      { std::terminate(); }

   string preenchido = barra.substr(a, b - a);
   string vacuo = barra.substr(b, c - (b + 1));
   string porcentos = barra.substr(c);

   // Separador que "delimita" desenho do progresso.
   mvaddstr(y, x, "[");

   /* Baseado no percentual, que indica onde está progresso, determinamos
    * as cores da parte preenchida. */
   if (phase < 25.0) 
      color_set((int)Vermelho, NULL);
   else if (phase >= 25.0 && phase < 50.0)
      color_set(Violeta, NULL);
   else if (phase >= 50.0 && phase < 75.0)
      color_set(Verde, NULL);
   else
      color_set(AzulMarinho, NULL);
   mvaddstr(y, x + a, preenchido.c_str());

   // O vázio terá sempre a mesma cor branca/amarela.
   color_set(Branco, NULL);
   mvaddstr(y, x + b, vacuo.c_str());

   // O outro separador que "delimita" desenho do progresso.
   mvaddstr(y, x + c - 1, "]");

   /* O percentuall está sublinhado. */
   /* O mais um, diz sobre pular o espaço em branco entre o progresso e 
    * o percentual. */
   attron(A_DIM);
   mvaddstr(y, x + c, porcentos.c_str());
   attroff(A_DIM);
}

bool Progresso::finalizado()
/* Apenas verifica se a taxa atual atingiu, ou até passou o total. */
   { return (this->atual >= this->total ); }

size_t Progresso::largura() { 
/* O algoritmo é o seguinte, obtém o percentual e o comprimento, usar o
 * 'gerador de rascunho' para gerar um rascunho, que é uma string no final,
 * logo mede este comprimento, e retorna tal valor. */
   double p = this->percentual();
   uint8_t len = this->comprimento; 
   string fmt_final = rascunha_barra(p, len);

   return fmt_final.length() + MARGEM;
}

/* === === ===  === === === === === === === === === === === === === === ===
 *                         Serialização 
 *                            e Deserialização
 * === === ===  === === === === === === === === === === === === === === = */
auto Progresso::serializa() -> queue<uint8_t> 
{
   const int sz = sizeof(size_t);
   const int N = 2 * sz;

   auto a = auxiliar::serializa_inteiro(this->atual);
   auto t = auxiliar::serializa_inteiro(this->total);
   // Com capacidade de 16 bytes inicialmente, prá evitar amortização.
   queue<uint8_t> output;

   auxiliar::anexa_fila(std::move(a), output);
   auxiliar::anexa_fila(std::move(t), output);

   // Verifica se operação foi bem sucedida...
   assert (output.size() == N);
   return output;
}

Progresso Progresso::deserializa(queue<uint8_t>& In)
{
   assert (In.size() == 16);

   size_t a = auxiliar::deserializa_inteiro(In);
   size_t t = auxiliar::deserializa_inteiro(In);

   assert (In.empty());
   return Progresso{a, t, 40};
}

#ifdef __UT_PROGRESSO__
/* === === ===  === === === === === === === === === === === === === === ===
 *                      Testes Unitários
 * === === ===  === === === === === === === === === === === === === === = */
#include <array>
#include <cstdarg>
// Biblioteca externa:
extern "C" {
  #include "teste.h"

  void executa_testes   (int, ...);
  void executa_testes_a (bool, int, ...);
}

using std::array;
using auxiliar::print_queue;


void teste_do_progresso(void)
{
   Progresso bar(5, 100, 30);
   int colunas[] = { 42, 8, 16, 32, 23, 13, 21, 37, 25, 2 };

   // Aumentando uns vinte, por este incremento.
   for (int i = 1; i <= 20; i++, bar+= 1);
   cout << bar << endl;

   auto janela = initscr();

   // configuração básica dela:
   cbreak();
   noecho();

   // Desenhando na grade de exibição ...
   uint8_t y = 1;
   do {
      auto x = colunas[y - 1];
      auto ponto = Posicao(2 * y, x);

      // Apaga tudo...
      erase();
      // Reeescreve a borda...
      box(janela, 0, 0);
      // Desenha o progresso novamente...
      bar.desenha(ponto);
      // Atualizando em 5 de uma vez só.
      bar += 5;
      refresh();
      // Taxa de velocidade do frame.
      napms(800);
   } while (y++ < 10);

   refresh();
   endwin();
}

void operador_cout_simples_teste(void)
{
   array<size_t, 4> totais = {800, 15, 350012, 1};

   for (int k = 0; k < 4; k++) {
      size_t T = totais[k];
      Progresso result{T / 2, T};
      cout << k << "ª. " << result << endl;
   }
}

void amostras_de_rascunhos_de_barras(void)
{
   const int C = 62;
   for (int k = 1; k <= 10; k++)
      cout << rascunha_barra(77.3 / k, C) << endl;
}

void serializacao_e_deserializacao_i(void)
{
   Progresso a(20), b(50), c(100);

   cout << a << endl << b << endl << c << endl;

   auto d = a.serializa();
   auto e = b.serializa();
   auto f = c.serializa();

   print_queue(d);
   print_queue(e);
   print_queue(f);

   auto g = Progresso::deserializa(d);
   auto h = Progresso::deserializa(e);
   auto i = Progresso::deserializa(f);
   
   cout << g << endl << h << endl << i << endl;
}

void serializacao_com_valores_nao_zerados(void) 
{
   Progresso a(50321), b(6000781), c(SIZE_MAX / 30);

   a += 14839;
   b += (size_t)(UINT32_MAX / 3000);
   c += SIZE_MAX / 500;
   cout << a << endl << b << endl << c << endl;

   auto d = a.serializa();
   auto e = b.serializa();
   auto f = c.serializa();

   auxiliar::print_queue(d);
   auxiliar::print_queue(e);
   auxiliar::print_queue(f);

   auto g = Progresso::deserializa(d);
   auto h = Progresso::deserializa(e);
   auto i = Progresso::deserializa(f);
   
   cout << g << endl << h << endl << i << endl;
}

int main(void) {
   executa_testes_a(
      true, 5,
      teste_do_progresso, false,
      operador_cout_simples_teste, true,
      amostras_de_rascunhos_de_barras, true,
      serializacao_e_deserializacao_i, true,
      serializacao_com_valores_nao_zerados, true
   );
}
#endif


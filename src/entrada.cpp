#include "entrada.hpp"

using namespace std;

const string ESPACO("  ");
// Comprimento da barra de progresso.
const int COMPRIMENTO = 28;



Entrada::Entrada(const string rotulo, size_t atual, size_t total) 
{ 
   this->rotulo = rotulo; 
   this->atual = atual; 
   this->total = total; 
}

static string comando_fortune_string(void) {
/* Faz da string do programa 'fortunate' um rótulo de amostra para testar
 * o programa que está se criando. */
   const int MAX = 500;
   FILE* saida;
   char buffer[MAX];
   int lido;

   saida = popen("fortune -n 170", "r");
   lido = fread(buffer, sizeof(char), MAX, saida);
   pclose(saida);

   // Substituindo quebras de linhas por motivo de compatibilidade.
   for (int q = 0; q < lido; q++) {
      char a = buffer[q];

      if (a == '\n' || a == '\t' || a == '\b' || a == '\r')
         buffer[q] = '*';
   }

   return string(buffer);
}

Entrada::Entrada(void) {
/* Usa o construtor 'default' para criar uma amostra, isso, ao menos na
 * versão de 'debug' deste programa. */
   using Distribuicao = uniform_int_distribution<size_t>;

   random_device motor;
   Distribuicao seletor(0, 300);

   this->rotulo = comando_fortune_string();
   this->total = seletor(motor);
   this->atual = seletor(motor);
   
   while (this->atual > this->total)
      (*this).atual = seletor(motor);
}


void Entrada::desenha(WINDOW* janela, int linha)
{
// Constrói a formatação do ítem na janela do ncurses.
   ostringstream fmt_a, fmt_bar, fmt_label;
   // Comprimento da barra de progresso.
   const int COMPRIMENTO = 28;
   // Total desta barra que foi carregado.
   int QUANTIA = (int)(COMPRIMENTO * this->percentual());
   int total_digitos = (int)(floor(log10(this->total)) + 1);

   // Criando a formatação da parte que diz sobre a quantia total/e a atual.
   fmt_a.width(total_digitos);
   fmt_a << (int)this->atual;
   fmt_a << "/";
   fmt_a << (int)this->total; 
   fmt_a << ESPACO.c_str();
   /* Criando uma formatação de barra de progresso em sí. */
   fmt_bar << '[';
   for (int n = 1; n < COMPRIMENTO; n++) {
      if (n < QUANTIA)
         fmt_bar << 'o';
      else
         fmt_bar << '.';
   }
   fmt_bar << ']';
   fmt_bar.precision(3);
   fmt_bar.width(5);
   fmt_bar << this->percentual() * 100.0; 
   fmt_bar << '%';
   // Rótulo. Tenta encurtar se for longo demais.
   if (this->rotulo.length() > 50) {
      int count = 1;
      for (char& letra: this->rotulo) {
         if (count++ > 50)
            break;
         fmt_label << letra;
      }
      fmt_label << ESPACO;
      fmt_label << "...";
      fmt_label << ESPACO;
   } else 
      fmt_label << this->rotulo;

   // Desenha tudo formatado acima na telinha do ncurses.
   move(linha, 2);
   addstr(fmt_label.str().c_str());
   move(linha, COLS - (fmt_a.str().length() + fmt_bar.str().length() + 2));
   addstr(fmt_a.str().c_str());
   addstr(fmt_bar.str().c_str());
}

float Entrada::percentual(void) {
/* Computa percentual já crescido da entrada. */
   float a = this->atual;
   float t = this->total;

   return a / t;
}

void Entrada::taxa_de_crescimento_a(void) {
/* Está taxa de crescimento leva em conta, intervalos de progressos definidos
 * para acionar uma nova velocidade de crescimento, que provavelmente é 
 * menor que a anterior. */
   // Percentual que ainda falta para terminar.
   float p = this->percentual();
   float P = 1.0 - p;
   // Quantia que tal "percentual faltante" significa.
   size_t T       = this->total;
   size_t R       = (size_t)(P * T);
   size_t um      = (size_t)(T * 0.01);
   size_t meio    = (size_t)(T * 0.005);
   size_t decimo  = (size_t)(R * 0.10);
   size_t quinto  = (size_t)(R * 0.20);
   size_t quarto  = (size_t)(R * 0.25);
   size_t terco   = (size_t)(R * 0.333);

   if (p < 0.20 && terco > 0)
      this->atual += terco;
   else if (p >= 0.20 && p < 0.40 && quarto > 0)
      this->atual += quarto;
   else if (p >= 0.40 && p < 0.60 && quinto > 0)
      this->atual += quinto;
   else if (p >= 0.60 && p < 0.90 && decimo > 0)
      this->atual += decimo;
   else if (p >= 0.90 && p < 0.95 && um > 0)
      this->atual += um;
   else if (p >= 0.95 && p < 0.98 && meio > 0)
      this->atual += meio;
   else 
      this->atual++;
}

void Entrada::taxa_de_crescimento_b(void) {
/* Está derivada aplica a maior taxa de crescimento que pode haver, e se ela
 * se esgotar, aplicar uma de menor intensidade. */
   // Percentual que ainda falta para terminar.
   float p = this->percentual();
   float P = 1.0 - p;
   // Quantia que tal "percentual faltante" significa.
   size_t T       = this->total;
   size_t R       = (size_t)(P * T);
   size_t um      = (size_t)(T * 0.01);
   size_t meio    = (size_t)(T * 0.005);
   size_t decimo  = (size_t)(R * 0.10);
   size_t quinto  = (size_t)(R * 0.20);
   size_t quarto  = (size_t)(R * 0.25);
   size_t terco   = (size_t)(R * 0.333);

   if (terco > 0)
      this->atual += terco;
   else if (quarto > 0)
      this->atual += quarto;
   else if (quinto > 0)
      this->atual += quinto;
   else if (decimo > 0)
      this->atual += decimo;
   else if (um > 0)
      this->atual += um;
   else if (meio > 0)
      this->atual += meio;
   else 
      this->atual++;
}

Entrada& Entrada::operator++(){
/* Aumenta uma fração do percentual do que ainda é restante. Uma ferramenta
 * feita especialmente para debug, fazer tais entradas aumentarem de forma
 * arbitraria para que possa debuggar sua interface gráfica. */
   // Abandona se já estiver finalizado o progresso.
   if (this->percentual() > 1.0) return *this;

   (*this).taxa_de_crescimento_b();

   // Correção de excedências, se houve um acrescimo acima do máximo(cem).
   if ((*this).atual > (*this).total)
      (*this).atual = (*this).total;

   return *this;
}

string Entrada::to_string(void) {
// Transforma a formatação numa string.
   ostringstream fmt_number, fmt_bar, fmt_label;
   // Total desta barra que foi carregado.
   int QUANTIA = (int)(COMPRIMENTO * this->percentual());
   int total_digitos = (int)(floor(log10(this->total)) + 1);

   // Criando a formatação da parte que diz sobre a quantia total/e a atual.
   fmt_number.width(total_digitos);
   fmt_number << (int)this->atual;
   fmt_number << "/";
   fmt_number << (int)this->total; 
   fmt_number << ESPACO.c_str();
   /* Criando uma formatação de barra de progresso em sí. */
   fmt_bar << '[';
   for (int n = 1; n < COMPRIMENTO; n++) {
      if (n < QUANTIA)
         fmt_bar << 'o';
      else
         fmt_bar << '.';
   }
   fmt_bar << ']';
   fmt_bar.precision(4);
   fmt_bar.width(4);
   fmt_bar << this->percentual() * 100.0; 
   fmt_bar << '%';
   // Rótulo. Tenta encurtar se for longo demais.
   if (this->rotulo.length() > 50) {
      int count = 1;
      for (char& letra: this->rotulo) {
         if (count++ > 50)
            break;
         fmt_label << letra;
      }
      fmt_label << ESPACO;
      fmt_label << "...";
      fmt_label << ESPACO;
   } else 
      fmt_label << this->rotulo;
   
   string Output(fmt_label.str());

   Output += ESPACO;
   Output += fmt_number.str();
   Output += fmt_bar.str();
   return Output;
}

ostream& operator<<(ostream& Output, Entrada& obj) {
// Formatação do tipo de dado na saída padrão, ou em que está acoplada a ela.
   Output << obj.to_string();
   return Output;
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                         Encapsulamento
 * == == == == == == == == == == == == == == == == == == == == == == == == */
string& Entrada::getRotulo(void) 
   { return this->rotulo; }

size_t Entrada::getTotal(void) 
   { return this->total; }

size_t Entrada::getAtual(void) 
   { return this->atual; }

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Serialização e Deserialização
 * == == == == == == == == == == == == == == == == == == == == == == == == */
#include <memory>
#include <climits>

static void serializa_u16(uint16_t In, uint8_t* Out) {
   const int sz = sizeof(uint16_t);
   uint8_t* bytes = reinterpret_cast<uint8_t*>(&In);

   uninitialized_copy_n(bytes, sz, Out);
}

static void serializa_usize(size_t In, uint8_t* Out) {
   const int sz = sizeof(size_t);
   uint8_t* bytes = reinterpret_cast<uint8_t*>(&In);

   uninitialized_copy_n(bytes, sz, Out);
}

static void serializa_str(const char* In, int In_a, uint8_t* Out) {
   auto In_b = const_cast<char*>(In); 
   auto In_c = reinterpret_cast<uint8_t*>(In_b);

   uninitialized_copy_n(In_c, In_a, Out);
}

static void insere_na_fila(uint8_t* In, int In_sz, queue<uint8_t>& Out) {
   int size = In_sz;

   for (int i = 1; i <= size; i++)
      Out.push(In[i - 1]);
}

queue<uint8_t> Entrada::serializa(void) {
   queue<uint8_t> Out;
   // O comprimento total da série de bytes é algo como o comprimento da 
   // string, mais 16 bytes referentes aos valores de inicio e fim, ambos
   // 8 bytes prá cada.
   int N, quantia = this->rotulo.size() + 2 * 8;
   // Quinhentos caractéres prá string no rótulo, uma coisa maior que 
   // isso seria até desnecessário pra aplicação.
   const int MAX = 2 * UCHAR_MAX;
   uint8_t buffer[MAX];

   // Copia os bytes, que indicam a quantidade total de bytes do objeto.
   N = sizeof(size_t);
   serializa_usize(quantia, buffer);
   insere_na_fila(buffer, Out);

   /* Copia a string. Na verdade os bytes(dois) com o comprimento dela, então
    * seu buffer interno. 
    * Transforma o valor num inteiro positivo de 16-bits, então pega seus 
    * bytes.*/
   N = sizeof(uint16_t);
   serializa_u16(this->rotulo.length(), buffer);
   insere_na_fila(buffer, Out);
   // Agora o buffer/data da string.
   N = this->rotulo.length() * sizeof(char);
   serializa_str(this->rotulo.c_str(), N, buffer);
   insere_na_fila(buffer, Out);

   // Serializa e colocar os valores 'atual' e 'total'.
   N = sizeof(size_t);
   serializa_usize(this->atual, buffer);
   insere_na_fila(buffer, N, Out);
   serializa_usize(this->total, buffer);
   insere_na_fila(buffer, N, Out);
   
   return Out;
}

Entrada Entrada::deserializa(std::queue<uint8_t>) 
   { Entrada x; return x; }


#ifdef __unit_tests__
#ifdef __linux__
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                   Testes Unitários de Entrada
 * == == == == == == == == == == == == == == == == == == == == == == == == */
 #include <iostream>

template <typename T>
void print_queue(queue<T>& fila) {
   int total = fila.size();

   cout << '(' << fila.size() << ") " << '[';
   while (total-- > 0) {
      auto X = fila.front();

      fila.pop();
      cout << (int)X << ", ";
      fila.push(X);
   }
   cout << "\b\b" << ']' << endl;
}

int main(int qtd, char* args[], char* vars[]) 
{
   Entrada a, b;

   cout << a << endl << b << endl;

   auto bytes_a = a.serializa();
   auto bytes_b = b.serializa();

   print_queue(bytes_a);
   print_queue(bytes_b);
}
#endif
#endif

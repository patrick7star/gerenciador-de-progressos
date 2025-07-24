#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <iostream>
#include <new>
// GNU library of POSIX:
#include <sys/types.h>
#include <unistd.h>

using namespace std;

const string ESPACO("  ");
// Comprimento da barra de progresso.
const int COMPRIMENTO = 28;
#define ig std::ignore


static int digitos_necessarios(size_t X)
// Computa o total de dígitos que o valor passado ocupa.
   {  return (int)(floor(log10(X)) + 1); }

bool Entrada::esgotado(void) const
/* Diz que o progresso de uma 'entrada' está esgotado, quando a 'atual' taxa
 * excede ou igualá o 'total'. */
   { return ((*this).atual == (*this).total); }
      
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                     Construturos e Destrutores
 * == == == == == == == == == == == == == == == == == == == == == == == == */
Entrada::Entrada(const string rotulo, size_t atual, size_t total, pid_t ID) 
{ 
   this->rotulo = rotulo; 
   this->atual = atual; 
   this->total = total; 
   this->id = ID;
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
   this->id = getpid();
   
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
   // int total_digitos = (int)(floor(log10(this->total)) + 1);
   int total_digitos = digitos_necessarios(this->total);

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

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                     Métodos Auxiliares Internos 
 * == == == == == == == == == == == == == == == == == == == == == == == == */
float Entrada::percentual(void) const {
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

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Operadores de Sobrecarga 
 * == == == == == == == == == == == == == == == == == == == == == == == == */
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

bool Entrada::operator==(Entrada& obj) const { 
   size_t atual, total;
   pid_t id;
   tie(atual, total, id) = obj.getIntAtributos();

   return (
      // Os totais deles tem que ser iguais.
      this->total == total    &&
      /* A taxa de finalização devem ser maior o igual. Tal condição, elimina
       * mesma entrada, porém com valores antigos. */
      this->atual >= atual    &&
      // Tem que pertencer ao mesmo processo.
      this->id == id 
   );
}

string Entrada::to_string(void) const {
// Transforma a formatação numa string.
   ostringstream fmt_number, fmt_bar, fmt_label;
   // Total desta barra que foi carregado.
   int QUANTIA = (int)(COMPRIMENTO * this->percentual());
   // int total_digitos = (int)(floor(log10(this->total)) + 1);
   int total_digitos = digitos_necessarios(this->total);

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
      for (const char& letra: this->rotulo) {
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

ostream& operator<<(ostream& Output, const Entrada& obj) {
// Formatação do tipo de dado na saída padrão, ou em que está acoplada a ela.
   const string auxiliar = obj.to_string();
   auto p = auxiliar.find("/", 0);
   auto rotulo = auxiliar.substr(0, p - 6);
   auto q = auxiliar.find("[", 0);
   auto t = auxiliar.find("]", q);
   auto barra = auxiliar.substr(q, t + 5);
   auto m = auxiliar.find("/");
   auto n = digitos_necessarios(obj.getTotal());
   auto l = ESPACO.length();
   auto info = auxiliar.substr(m - n, l + 2 * n);

   pid_t id;
   tie(ig, ig, id) = obj.getIntAtributos();

   Output << "PID: " << id << endl;
   Output << "Rótulo: \"" << rotulo  << '\"'<< endl;
   Output << "Barra: " << barra << endl;
   Output << "Info: " << info << ESPACO << obj.getAtual() << " e " 
          << obj.getTotal() << endl << endl;

   return Output;
}

ostream& operator<<(ostream& Output, Entrada& obj) {
// Formatação do tipo de dado na saída padrão, ou em que está acoplada a ela.
   const string auxiliar = obj.to_string();
   auto p = auxiliar.find("/", 0);
   auto rotulo = auxiliar.substr(0, p - 6);
   auto q = auxiliar.find("[", 0);
   auto t = auxiliar.find("]", q);
   auto barra = auxiliar.substr(q, t + 5);
   auto m = auxiliar.find("/");
   auto n = digitos_necessarios(obj.getTotal());
   auto l = ESPACO.length();
   auto info = auxiliar.substr(m - n, l + 2 * n);

   pid_t id;
   tie(ig, ig, id) = obj.getIntAtributos();

   Output << "PID: " << id << endl;
   Output << "Rótulo: \"" << rotulo  << '\"'<< endl;
   Output << "Barra: " << barra << endl;
   Output << "Info: " << info << ESPACO << obj.getAtual() << " e " 
          << obj.getTotal() << endl << endl;

   return Output;
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                         Encapsulamento
 * == == == == == == == == == == == == == == == == == == == == == == == == */
const string& Entrada::getRotulo(void) const
   { return this->rotulo; }

size_t Entrada::getTotal(void) const
   { return this->total; }

size_t Entrada::getAtual(void) const
   { return this->atual; }

tuple<size_t, size_t, pid_t>
Entrada::getIntAtributos(void) const {
   auto atual = this->atual;
   auto total = this->total;
   auto id = this->id;

   return make_tuple(atual, total, id);
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                      Serialização e Deserialização
 * == == == == == == == == == == == == == == == == == == == == == == == == */
#include <memory>
#include <climits>

Bytes Entrada::serializa(void) {
/*   Sem sucesso ainda com um tipo serialização dinâmica, tentarei ao invés 
 * a versão estática. Assim, o retorno ao invés de uma fila, será uma array
 * fixa, com os bytes todos enfileirados. A ordem de inserção, ainda continua
 * a mesma que a anterior: em primeiro, os bytes dos valores com tamanhos 
 * fixos em tempo de compilação; depois os tipos de dados variaveis, que 
 * neste caso é essencialmente a string. O tamanho de tal array é definida
 * pela constante 'MAX_SERIALIZACAO'.
 *   Então a organização dos dados serializados fica: na parte estática, 
 * atributos 'atual' e 'total', nesta ordem; já a parte dinâmica fica o 
 * restante, como é a única, tal fase é extremamente fácil, o que sobra
 * do buffer, apenas é preenchidos com caractéres nulos. */
   array<uint8_t, MAX_SERIAL> Out;
   int escrito = 0, sz = 0;
   uint8_t* fonte{nullptr}, *destino{nullptr};

   // Preenche com zeros, principalmente para a string que será escrita.
   uninitialized_fill(Out.begin(), Out.end(), 0x0);
   // Serialização e copia do inteiro sem sinal de máquina 'atual'.
   sz = sizeof(size_t);
   fonte = reinterpret_cast<uint8_t*>(&this->atual);
   destino = Out.begin() + escrito;
   uninitialized_copy_n(fonte, sz, destino);
   escrito += sz;
   // Serialização e copia do inteiro sem sinal de máquina 'total'.
   fonte = reinterpret_cast<uint8_t*>(&this->total);
   destino = Out.begin() + escrito;
   uninitialized_copy_n(fonte, sz, destino);
   escrito += sz;
   // Serialização do PID da 'entrada'. De que processo ela pertence. 
   sz = sizeof(pid_t);
   fonte = reinterpret_cast<uint8_t*>(&this->id);
   destino = Out.begin() + escrito;
   uninitialized_copy_n(fonte, sz, destino);
   escrito += sz;
   // Serialização dos dados da 'string' do atributo 'rótulo'.
   auto quantia = this->rotulo.length();
   sz = sizeof(char);
   auto aux = const_cast<char*>(this->rotulo.c_str());
   fonte = reinterpret_cast<uint8_t*>(aux);
   destino = Out.begin() + escrito;
   uninitialized_copy_n(fonte, quantia * sz, destino);
   escrito += quantia * sz;

   return Out;
}

size_t deserializa_sizet(Bytes& In, int* cursor) {
   auto bytes = In.data();
   size_t* pointer = (size_t*)(bytes + *cursor);
   auto valor = *pointer;
   const int sz = sizeof(size_t);

   (*cursor) += sz;
   return valor;
}

string deserializa_string(Bytes& In, int* cursor) {
   auto restante = In.size() - (*cursor + 1); 
   auto rawptr = In.data();
   auto str = reinterpret_cast<char*>(rawptr);
   auto buffer = new char[restante];
   string Output;
   auto fonte = str + *cursor;

   uninitialized_copy_n(fonte, restante, buffer);
   Output = string(buffer);
   free(buffer);
   (*cursor) += restante;

   return Output;
}

pid_t deserializa_pidt (Bytes& In, int* cursor) {
   auto bytes = In.data();
   pid_t* pointer = (pid_t*)(bytes + *cursor);
   auto valor = *pointer;
   const int sz = sizeof(pid_t);

   (*cursor) += sz;
   return valor;
}

Entrada Entrada::deserializa(Bytes& In) { 
   int lido = 0;
   auto atual = deserializa_sizet(In, &lido);
   auto total = deserializa_sizet(In, &lido);
   auto id = deserializa_pidt(In, &lido);
   auto rotulo = deserializa_string(In, &lido);

   if (total < 0)
      throw invalid_argument("bytes inválidos");

   return Entrada(rotulo, atual, total, id);
}


#ifdef __unit_tests__
#ifdef __linux__
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                   Testes Unitários de Entrada
 * == == == == == == == == == == == == == == == == == == == == == == == == */
 #include <iostream>
 #include <queue>
 #include <cassert>

template <typename T>
static void print_queue(queue<T>& fila) {
/* Realiza a impressão dos itens de tipo 'T' na 'fila'. Isso não é 'TS',
 * porque ele modifica o objeto para realizar tal tarefa, então cuidado. */
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

template <typename T>
static void print_array(array<T, MAX_SERIAL>& seq) {
   cout << '(' << seq.size() << ") " << '[';
   for (T& item: seq) 
      cout << (int)item << ", ";
   cout << "\b\b" << ']' << endl;
}

int main(int qtd, char* args[], char* vars[]) 
{
   Entrada a, b;

   cout << "pid_t: " << sizeof(pid_t) << " bytes\n";
   cout << a << endl << b << endl;

   auto bytes_a = a.serializa();
   auto bytes_b = b.serializa();

   print_array(bytes_a);
   print_array(bytes_b);

   auto A = Entrada::deserializa(bytes_a);
   auto B = Entrada::deserializa(bytes_b);

   cout << endl << A << endl << B << endl;
   assert (A == a);
   assert (B == b);
}
#endif
#endif

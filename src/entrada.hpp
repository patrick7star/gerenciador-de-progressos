#pragma once
// Bibliotecas externas:
#include <curses.h>
// Biblioteca padrão do C++:
#include <random>
#include <sstream>
#include <string>
#include <array>
#include <tuple>
#include <chrono>
// Bibliotecas legadas do C:
#include <cmath>
// Biblioteca Glibc:
#include <sys/types.h>
#include <unistd.h>

constexpr int MAX_SERIAL = 300;
// Saída e entrada dos métodos de serialização.
using Bytes = std::array<uint8_t, MAX_SERIAL>;
using AtributosInt = std::tuple<size_t, size_t, int>;

class Entrada {
/* Basicamente o progresso em sí. Está estrutura obtém os valores que está
 * atualmente, e o que precisa ser atingido. Também permite a visualização
 * pura e no 'ncurses', do objeto. Cronometra quando se passou de cada 
 * entrada individual. 
 */
 using Cloque = std::chrono::system_clock;
 using TimePoint = std::chrono::time_point<Cloque>;
 using AtributosTP = std::tuple<TimePoint, TimePoint, bool>;

 private:
   // Descrição de um progresso que está rodando no painel.
   std::string rotulo;

   // Os valores atuais e total a ser atingindo pelo progresso.
   size_t atual{0};
   size_t total;

   // Processo da instância da entrada:
   pid_t id;

   // Relógio para medir o tempo de começo e termino da 'entrada', sem falar
   // de outras coisas
   TimePoint inicio; 
   TimePoint termino;
   // Diz se ambos 'selos de tempos' ainda não foram marcados.
   bool marcados{false};

   // Dois tipos de crescimento da 'Entrada' pra debugging.
   void taxa_de_crescimento_a(void);
   void taxa_de_crescimento_b(void);
   void registra_o_termimo(void);

 public:
   Entrada(const std::string r, size_t a, size_t t, pid_t id, TimePoint ti, 
     TimePoint tf, bool m);
   // Método construído para debug. Os dados são totalmente arbitrários.
   Entrada(void);
   Entrada(const std::string r, size_t a, size_t t)
     : rotulo(r), atual(a), total(t)
   { 
      this->id = getpid(); 
      this->inicio = Cloque::now(); 
      this->termino = Cloque::now();
   }

   // Percentual do atual progresso da 'entrada'.
   float percentual(void) const;
   // Faz o desenho da 'entrada' no 'ncurses'.
   void desenha(WINDOW* janela, int linha);
   // Converte sua formatação numa string.
   std::string to_string() const;
   // Informa se tal entrada se esgotou.
   bool esgotado(void);

   // Encapsulamento do objeto.
   const std::string& getRotulo(void) const;
   size_t getTotal(void) const;
   size_t getAtual(void) const;
   // Método da uma sintaxe de todos os inteiros internos da instância.
   // Ele retorna o 'atual', 'total' e 'pid' inteiro, nesta ordem.
   AtributosInt getIntAtributos(void) const;
   AtributosTP getTPAtributos(void) const;

   Entrada& operator++();
   Entrada& operator++(int);
   Entrada& operator=(size_t atual); 
   Entrada& operator+=(size_t qtd); 

   // Processo de serialização do tipo.
   auto serializa(void) -> Bytes;
   static auto deserializa(Bytes&) -> Entrada;
};

 // Função bastante útil de conversão de booleanos.
 constexpr const char* const BoolStr(bool valor)
   { return valor ? "true": "false"; }
 
 /* Verifica se a 'entrada(a)' é igual à 'entrada(b)', verificando se o
  * valor 'atual' dela também é mais adiantado do que a de 'b'. */
 bool operator>=(Entrada& a, Entrada& b); 
 bool operator==(Entrada& a, Entrada& b);

/* Formatação da 'Entrada' na saída padrão, é quase que a mesma que é exibida
 * porém, com um toque de debug, fazendo assim algo não tão útil. */
 std::ostream& operator<<(std::ostream& Output, Entrada& Obj); 

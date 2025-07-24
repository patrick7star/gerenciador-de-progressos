#pragma once
// Bibliotecas externas:
#include <curses.h>
// Biblioteca padrão do C++:
#include <random>
#include <sstream>
#include <string>
#include <array>
#include <tuple>
// Bibliotecas legadas do C:
#include <cmath>
// Biblioteca Glibc:
#include <sys/types.h>
#include <unistd.h>

constexpr int MAX_SERIAL = 2000;
// Saída e entrada dos métodos de serialização.
using Bytes = std::array<uint8_t, MAX_SERIAL>;
using AtributosInt = std::tuple<size_t, size_t, int>;

class Entrada {
 private:
   // Descrição de um progresso que está rodando no painel.
   std::string rotulo;

   // Os valores atuais e total a ser atingindo pelo progresso.
   size_t atual = 0;
   size_t total;

   // Processo da instância da entrada:
   pid_t id;

   // Dois tipos de crescimento da 'Entrada' pra debugging.
   void taxa_de_crescimento_a(void);
   void taxa_de_crescimento_b(void);

 public:
   Entrada(const std::string r, size_t a, size_t t, pid_t id);
   // Método construído para debug. Os dados são totalmente arbitrários.
   Entrada(void);
   Entrada(const std::string r, size_t a, size_t t)
     : rotulo(r), atual(a), total(t)
      { this->id = getpid(); }

   // Percentual do atual progresso da 'entrada'.
   float percentual(void) const;
   // Faz o desenho da 'entrada' no 'ncurses'.
   void desenha(WINDOW* janela, int linha);
   // Converte sua formatação numa string.
   std::string to_string() const;
   // Informa se tal entrada se esgotou.
   bool esgotado(void) const;

   // Encapsulamento do objeto.
   const std::string& getRotulo(void) const;
   size_t getTotal(void) const;
   size_t getAtual(void) const;
   // Método da uma sintaxe de todos os inteiros internos da instância.
   // Ele retorna o 'atual', 'total' e 'pid' inteiro, nesta ordem.
   AtributosInt getIntAtributos(void) const;

   Entrada& operator++();
   bool operator==(Entrada& obj) const;

   // Processo de serialização do tipo.
   auto serializa(void) -> Bytes;
   static auto deserializa(Bytes&) -> Entrada;
};

/* Formatação da 'Entrada' na saída padrão, é quase que a mesma que é exibida
 * porém, com um toque de debug, fazendo assim algo não tão útil. */
 std::ostream& operator<<(std::ostream& Output, Entrada& Obj); 
/* Pega uma 'fila com bytes', que é gerada pelo método de serialização do 
 * objeto, então compacta tal série de bytes com o comprimento, um inteiro
 * positivo de máquina(8 bytes), no começo. Retorna tal array dinâmicamente
 * alocada para o chamador. */
 // uint8_t* compacta_serializacao(std::queue<uint8_t>&& In);

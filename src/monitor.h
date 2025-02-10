#pragma once
#include <vector>
#include <string>
#include <array>
#include <chrono>
#include <filesystem>
#include "progresso.h"
#include "led.h"
#include "entrada.hpp"

/* Tipos de ordenação que se pode fazer na estrutura Tela, quando perdir
 * para ordenar as 'Entradas'. Os iniciais -- podem haver mais futuramente
 * -- são: criação do progresso(Criação), se está perto de finalizar(
 * Próximo do Fim), qual progresso mais cresce durante o tempo(Taxa de 
 * Crescimento), e o último é qual está mais tempo executando(Duração).
 *  )*/
enum class Ordenacao 
{
   // Ordenação na data de criação da entrada.
   Criacao, 

   // Ordenação baseado em quanto o progresso está perto ao fim.
   ProximoDoFim,

   // Ordenação baseado na taxa de crescimento dos progressos.
   TaxaDeCrescimento,

   // Ordenação baseado na duração do progresso até aqui.
   Duracao
};

// Cores com os determinados números, arbitrariamente escolhidos.
enum Cores { 
   Branco = 94, Violeta, Azul, Vermelho, 
   Verde, Amarelo, AzulMarinho
};

namespace monitor {
   // Algums apelidos, e uso de nomes, apenas neste escopo:
   using Clock = std::chrono::high_resolution_clock;
   using std::chrono::time_point;
   using std::string;
   using std::vector;
   using std::array;
   using std::queue;
   using std::filesystem::path;
   using entrada::Entrada;
   using entrada::EntradaPipe;

   class Tela {
   /* Onde realmente é gerenciado os progressos, estes contidos na estrutura
    * Entrada. O objeto executa a biblioteca gráfica ncurses, esta embutida
    * nele, e também a finaliza. É um "embrulho" dela, porém sem recorrer
    * a herança, e especializada em manipular Entradas. */
   private:
      // Instância da janela iniciada do 'ncurses'.
      WINDOW* janela;

      // Velocidade da atualização de quadros da tela.
      int VELOCIDADE = 800;

      // Progressos que estão sendo mostrados na tela no momento.
      vector<Entrada> lista;
      // Fila de entradas referentes a 'named pipes'.
      queue<EntradaPipe> fila;

      // Atual tipo de ordenação que tais entradas estão submetidas.
      Ordenacao ordem;

      /* Todos aquelas entradas estrangeiras que requisatarem acessar o 
       * Painel, precisam enviar a "EntradaPipe" inicialmente para cá, apenas
       * uma vez, depois de processado e inserido na fila, o canal próprio
       * dela que servirá de comunicação. */
      path canal_de_insercao; int fd;

      // Métodos deste tipo de objeto.
      void ordena();
      void desenha_status();

   public:
      // Construtores e desconstrutores:
      Tela();
      ~Tela();

      void adiciona(Entrada&& obj);
      void renderiza();
      bool tudo_finalizado();
      array<int, 2> dimensao(); 
      void altera_ordenacao();

      // Sobrecarga de alguns operadores:
      Entrada& operator[](string nome);
   };
}

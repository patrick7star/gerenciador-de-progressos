#pragma once
#include <vector>
#include <string>
#include <array>
#include <chrono>
#include <filesystem>
#include "progresso.h"
#include "led.h"

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

   class Entrada {
   /* É como o progresso, porém tocante só na parte visual, com um letreiro
    * dinâmico. */
   public:
      // Função ou nome do que o progresso representa.
      LED rotulo;

      /* Dados do referentes a onde está o progresso, e também sua 
       * representação gráfica. */
      Progresso bar;

      // Tempo de partida da criação.
      time_point<Clock> criacao;

      // Acrescimo do percentual à cada 400 milisegundos.
      double taxa;

      /* Construtor personalizado, com todos parâmetros existentes.
       *
       * Legendas:
       *    - label: mensagem que fica as vezes rotacionando, basicamente a 
       *             descrição do progresso em sí. 
       *    - a: valor que o progresso é registrado na lista.
       *    - t: total que é preciso atingir para o progresso terminar.
       *    - vC: comprimento do visor, que o rótulo fica girando em sí mesmo.
       *    - bC: comprimento da barra de progresso que é mostrada na tela.
       *    - tI: taxa do percentual de incrementação do progresso.
       *    - tC: timestamp do momento da criação deste progresso.
       */
      Entrada(string label, size_t a, size_t T, uint8_t vC, uint8_t bC,
        double tI, time_point<Clock> tC);
      Entrada (string label, size_t t);
      // Entrada (path caminho_pipe);
      Entrada();

      // Métodos que serializam e deserializam 'Entrada'.
      auto serializa() -> queue<uint8_t>;
      auto deserializa(queue<uint8_t>&) -> Entrada;

      /* Sobrecarga de operadores. Você pode comparar se uma entrada é menor
       * ou igual a outra. Também pode incrementar uma unidade ou mais 
       * o progresso interno dela. */
      bool operator<(Entrada& obj);
      bool operator>(Entrada& obj); 
      Entrada& operator+=(size_t q);
      Entrada& operator++();
   };

   class EntradaPipe: public Entrada {
   /* Uma instância da entrada que gera instâncias de forma dinâmica, todas
    * elas são recuperadas via named pipes. Isso nem é uma coisa apenas pra 
    * C/ou C++, será aceitos dados linguagens, se elas seguirem o protocolo
    * comum. */
   public:
      // Como agora é multiprocesso, será necessário registrar o 'pid';
      pid_t ID;
      /* O caminho do 'named pipe' é onde os dados serão lidos pelo o painel
       * geral, e onde o proprietário da 'EntradaPipe' envia atualizações
       * do estágio do tipo de dado. O inteiro armazena o identificador 
       * quando tal 'canal' for aberto. */
      path canal; int fd;

      // Operações extra para o tipo:
      bool operator==(EntradaPipe& obj);

      auto serializa() -> queue<uint8_t>;
      auto deserializa(queue<uint8_t>&) -> Entrada;
   };

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

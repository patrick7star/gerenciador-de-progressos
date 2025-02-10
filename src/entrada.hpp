#pragma once
// Biblioteca padrão do C:
#include <string>
#include <chrono>
#include <filesystem>
#include <ostream>
// Partes do próprio projeto:
#include "progresso.h"
#include "led.h"

namespace entrada {
  // Algums apelidos, e uso de nomes, apenas neste escopo:
  using Clock = std::chrono::high_resolution_clock;
  using std::chrono::time_point;
  using std::string;
  using std::queue;
  using std::filesystem::path;

  typedef class EntradaPipe EP;

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
   private:
      void abre_named_pipe_automatico();
      void cria_named_pipe_automatico();

   public:
      // Como agora é multiprocesso, será necessário registrar o 'pid';
      pid_t ID;
      /* O caminho do 'named pipe' é onde os dados serão lidos pelo o painel
       * geral, e onde o proprietário da 'EntradaPipe' envia atualizações
       * do estágio do tipo de dado. O inteiro armazena o identificador 
       * quando tal 'canal' for aberto. */
      path canal; int fd;

      // Métodos constrututores e descontrutores:
      EntradaPipe(
        pid_t id, path ch, int fd, string l, size_t a, size_t T, uint8_t vC,
        uint8_t bC, double tI, time_point<Clock> tC
      );
      EntradaPipe(path canal, string label, size_t t); 
      EntradaPipe(string l, size_t t)
        : EntradaPipe(path("tubulação"), l, t) {}
      EntradaPipe();
      ~EntradaPipe();

      // Operações extra para o tipo:
      bool operator==(EntradaPipe& obj);

      auto serializa() -> queue<uint8_t>;
      static auto deserializa(queue<uint8_t>&) -> EntradaPipe;

      // Envia nova atualização de dados.
      void atualiza_externo();
   };
}

// Sobrecarga para formatação dos tipos de dados acima:
std::ostream& operator<<(std::ostream& output, entrada::EntradaPipe& obj);
std::ostream& operator<<(std::ostream& output, entrada::Entrada& obj);

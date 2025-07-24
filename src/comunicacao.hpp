#ifndef __COMUNICACAO_H__
#define __COMUNICACAO_H__
#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <chrono>
#include <queue>
#include <vector>


class Servidor {
/*   Gerenciamento o envio de dados referentes à uma ou mais 'entradas'. Sabe
 * quando enviar, e parar de enviar, até pausar o processo de transmissão.
 * Também registra quantas vezes foi enviado, e o tanto de bytes que foi 
 * acumulado no total deste envio.
 */

 using Queue = std::queue<Entrada*>;
 using Clock = std::chrono::steady_clock;
 using TimePoint = std::chrono::steady_clock::time_point;

 private:
   // 'file descriptor' do 'named pipe', onde os dados são enviados.
   int tubulacao;
   // Referência da 'entrada' que sempre transmite seu sinal.
   // queue<Entrada*> fila;   
   Queue fila;   
   // Cronômetro que registra o tempo necessário prá próxima remessa.
   // mutable time_point<steady_clock> inicio;
   mutable TimePoint inicio;

   // Diz ao objeto se é hora de enviar.
   bool pronto_pra_envio(void) const;
   void envia_uma_entrada(Entrada& obj) const;
   bool entrada_pertencente(Entrada& obj);

 public:
   Servidor(void);
   ~Servidor(void); 

   // Envia todas as 'entradas' no 'pool'.
   void enviar(void); 
   // Adiciona 'entrada' pra ser transmitida, se não estiver finalizada.
   bool adiciona(Entrada* pointer); 
};


class Cliente {
/*   Receberá as 'entradas' que são enviadas via 'pipe'. Entretanto, ela tem
 * uma lista interna, que aceita apenas 'entradas' não repetidas, e que 
 * sejam atualizadas. */

 using Lista = std::vector<Entrada>;
 using Clock = std::chrono::steady_clock;
 using TimePoint = std::chrono::time_point<Clock>;

 private:
   // Viaduto(named pipe) de 'entradas' recebidas.
   int tubo;
   // Referência mutavel a lista de 'Entradas' externa.
   Lista& colecao;
   // Cronômetro prá contar até a próxima inserção.
   TimePoint inicio;
   TimePoint tempo;

   void insercao_controlada(Entrada& obj);
   void tenta_ler_entrada(Bytes& Out);
   bool permicao_de_leitura(void);

 public:
   /* O construtor deve receber uma lista, pra que se aloque todas entradas
    * externas que foram recebidas, mesmo que ela esteja incialmente vázia. 
    */
   Cliente(Lista& lista);
   ~Cliente(void); 

   /* Lê alguns bytes, e tenta decodifica-lô numa 'entrada'. Se for possível,
    * insere nova, ou apenas atualiza se necessário. */
   bool receber(void); 
};
#endif

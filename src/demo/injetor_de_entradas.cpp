#if defined(__linux__)
// Biblioteca padrão em C++:
#include <array>
#include <thread>
#include <iostream>
#include <chrono>
#include <new>
#include <iomanip>
// Antiga biblioteca do C:
#include <signal.h>
#include <cstdlib>
#include <cstring>
// Bibliotecas externas:
// GNU library of POSIX:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// Outros módulos:
#include "../entrada.hpp"

// Quantia de entradas criadas na compilação.
constexpr int QTD = 7;
constexpr int Failed = -1, Okay = 0;
// Canal único de transmissão de dados(prá fora).
static int SAIDA, CONSUMIDO = 0;


static void cria_tubulacao(void) {
   constexpr char NOME[] = "./tubulação";
   constexpr int Okay = 0, Failed = -1;
   constexpr int MODE = 0600;

   if (mkfifo(NOME, MODE) == Failed) {
      switch (errno) {
         case EEXIST:
            std::cout << "O named pipeline 'tubulação' já existe.\n"; 
            break;
         default:
            std::cout << "Tubulação criada com sucesso.\n";
      }
   }
}

static void envia_status_da_entrada(Entrada& obj, int tubo) {
   auto conteudo = obj.serializa();

   write(tubo, conteudo.data(), MAX_SERIAL);
}

static void fechamento(int sinal_enviado) {
   std::cout << "O named pipe está sendo fechado ... ";
   close(SAIDA);
   std::cout << "feito.\n";
   _exit(0);
}

static void informacao_do_buffer_do_named_pipe(void) {
   auto capacidade = fcntl(SAIDA, F_GETPIPE_SZ);
   auto consumido = static_cast<float>(CONSUMIDO);
   auto calculo = consumido / static_cast<float>(capacidade);

   std::setw(2);
   std::cout.precision(3);
   std::cout << "Total de utilização:" << calculo * 100.0 
      << '%' << std::endl << std::endl; 
}

static void tenta_limpa_o_buffer_cheio(void) {
   auto capacidade = fcntl(SAIDA, F_GETPIPE_SZ);
   std::array<uint8_t, MAX_SERIAL> lixo;

   if (CONSUMIDO > capacidade) {
      int entrada = SAIDA;

      // Também limpa o buffer para novas injeções.
      if (read(entrada, lixo.data(), MAX_SERIAL) == Failed) {
         auto code = errno;
         std::cout << strerror(code) << std::endl;

      } else {
         CONSUMIDO = 0;
         std::cout << "Todo buffer foi limpo com sucesso.\n";
      }
   }
}

static void gerencia_buffer(void) {
   constexpr auto ms = 700;
   constexpr auto PAUSA = std::chrono::milliseconds(ms);

   while (true) {
      tenta_limpa_o_buffer_cheio();
      informacao_do_buffer_do_named_pipe();
      std::this_thread::sleep_for(PAUSA);
   }
}

/* Por enquanto, apenas funciona para plataformas Linux. */
int main(int qtd, char* args[], char* vars[]) 
{
   constexpr int ms = 1100;
   constexpr auto PAUSA = std::chrono::milliseconds(ms);
   std::array<Entrada, QTD> inputs;
   std::thread manager(gerencia_buffer);

   signal(SIGINT, fechamento);
   cria_tubulacao();

   SAIDA = open("./tubulação", O_RDWR);

   do {
      for (auto& In: inputs) {
         std::cout << In << std::endl;
         envia_status_da_entrada(In, SAIDA);
         // Varia o valor da 'Entrada'.
         ++In;
         // Pausa para até à próxima remessa.
         std::this_thread::sleep_for(PAUSA);
         // Registra o que está sendo alocado no buffer.
         CONSUMIDO += MAX_SERIAL;
      } 
   } while(true);
}
#endif

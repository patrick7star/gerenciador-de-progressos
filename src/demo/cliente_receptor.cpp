#if defined(__linux__)
// Biblioteca padrão em C++:
#include <array>
#include <thread>
#include <iostream>
#include <chrono>
#include <new>
#include <tuple>
#include <vector>
// Antiga biblioteca do C:
// Bibliotecas externas:
// GNU library of POSIX:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// Outros módulos:
#include "../comunicacao.hpp"
#include "../entrada.hpp"

// Quantia de entradas criadas na compilação.
constexpr int QTD = 8;
constexpr int Failed = -1, Okay = 0;
// Meio de comunicação comum.
static int ENTRADA;


static bool todas_entradas_finalizadas(std::vector<Entrada>& In) {
   int contagem = 0;

   if (In.size() == 0)
   // Não abandonar se não houver entradas ainda. Então evalua lista vázias
   // como sem termino.
      return false;

   for (auto iter = In.begin(); iter != In.end(); iter++) {
      if (iter->esgotado()) 
         contagem++;
   }

   std::cout << "Qtd. de esgotadas: " << contagem << std::endl;
   return In.size() == contagem;
}

/* Por enquanto, apenas funciona para plataformas Linux. */
int main(int qtd, char* args[], char* vars[]) 
{
   using millis = std::chrono::milliseconds;
   using seg = std::chrono::seconds;
   using Clock = std::chrono::steady_clock;
   using TimePoint = Clock::time_point;

   constexpr auto PAUSA = millis(600);
   int contagem = 0, negadas = 0;
   std::vector<Entrada> lista;
   auto correspondencia = Cliente(lista);
   auto inicio = Clock::now();
   auto fim = inicio;
   auto decorrido = [](TimePoint i, TimePoint f)
      { return std::chrono::duration_cast<seg>(f - i); };
   bool tempo_nao_esgotado;

   do {
      auto inserido = correspondencia.receber();
      auto duracao = decorrido(inicio, fim);

      fim = Clock::now();
      tempo_nao_esgotado = duracao < millis(2000);

      if (!inserido) negadas++;

      std::cout << "Inserções negadas: " << negadas 
                << "\nQuantia listada: " << lista.size()
                << "\nEntradas expiradas: " << correspondencia.quantidade()
                << "\nTempo esgotado? " << BoolStr(!tempo_nao_esgotado)
                << std::endl;
      std::this_thread::sleep_for(PAUSA);

   } while(lista.size() > 0 || tempo_nao_esgotado);
}
#endif

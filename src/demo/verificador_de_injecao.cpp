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
#include "../entrada.hpp"

// Quantia de entradas criadas na compilação.
constexpr int QTD = 8;
constexpr int Failed = -1, Okay = 0;
static int INSERCOES_NEGADAS = 0;
// Meio de comunicação comum.
static int ENTRADA;


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

static Entrada recebe_status_da_entrada(void) {
   std::array<uint8_t, MAX_SERIAL> auxiliar;

   read(ENTRADA, auxiliar.data(), MAX_SERIAL);
   return Entrada::deserializa(auxiliar);
}

static bool todas_entradas_finalizadas(std::vector<Entrada>& In) {
   int contagem = 0;

   for (auto iter = In.cbegin(); iter != In.cend(); iter++) {
      if (iter->esgotado()) 
         contagem++;
   }

   std::cout << "Qtd. de esgotadas: " << contagem << std::endl;
   return In.size() == contagem;
}

static void insere(Entrada& In, std::vector<Entrada>& Out) {
   int cursor       = 0;
   int comprimento  = Out.size();

   for (int n = 1; n <= comprimento; n++) {
      Entrada& X = Out[n - 1];

      if (In == X) {
         auto insercoes = INSERCOES_NEGADAS;

         if (X.getAtual() < In.getAtual() && !Out.empty()) 
            Out[n - 1] = In; 

         std::cout << "Inserções negadas: " << insercoes << std::endl;
         ++INSERCOES_NEGADAS;
         return;
      }
      cursor++;
   }
   Out.push_back(In);
}

/* Por enquanto, apenas funciona para plataformas Linux. */
int main(int qtd, char* args[], char* vars[]) 
{
   using namespace std;
   using namespace chrono;

   constexpr int ms     = 600;
   constexpr auto PAUSA = milliseconds(ms);
   int contagem         = 0;
   vector<Entrada> lista;

   cria_tubulacao();
   ENTRADA = open("./tubulação", O_RDWR);

   do {
      try {
         auto entry = recebe_status_da_entrada();

         cout << endl << entry << endl;
         insere(entry, lista);
         cout << "Há " << lista.size() << " entradas na lista.\n";

      } catch(...) {
         cout << "Falha na leitura, decodificação e deserialização\n";
      }
      this_thread::sleep_for(PAUSA);

   } while(!todas_entradas_finalizadas(lista));

   close(ENTRADA);
}
#endif

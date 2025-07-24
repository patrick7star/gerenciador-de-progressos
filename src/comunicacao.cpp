#include "comunicacao.hpp"
// Outros módulos deste projeto:
#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <tuple>
#include <iostream>
// Biblioteca padrão do C:
#include <cerrno>
#include <cstring>
// API do sistema:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace chrono;
// Valores globais utilizados abaixo:
const string PATH{"./tubulação"};
constexpr int Failed = -1;
// Valores que tanto, o Servidor, como o Cliente, se comunicam.
constexpr auto RITMO_DE_ENTREGA = 200ms;
constexpr auto RITMO_DE_RECEPTACAO = 100ms;


static void cria_tubulacao(void) {
/* Tenta criar o 'named pipe' se não existir. Se já houver, apenas uma 
 * mensagem informando o error será emitida. */
   constexpr int MODE = 0600;

   if (mkfifo(PATH.c_str(), MODE) == Failed) {
      switch (errno) {
         case EEXIST:
            std::cerr << "O named pipeline 'tubulação' já existe.\n"; 
            break;
         default:
            std::cout << "Tubulação criada com sucesso.\n";
      }
   }
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                        Servidor(Envia as Entradas)
 *
 *  O servidor tem um ritmo que ele envia stream de bytes das 'entradas' que 
 * são passadas pra ele. O bom fosse que este fosse definido de forma
 * programatica, respeitando a taxa de crescimento do 'progresso' interno. 
 * Porém isso ainda é algo dífícil de codificar. Também ele não pode passar
 * a taxa de leitura do 'cliente', porque se encher demais sua caixa, 
 * os dados ficaram defazados.
 * == == == == == == == == == == == == == == == == == == == == == == == == */
bool Servidor::pronto_pra_envio(void) const 
{
   auto fim = steady_clock::now();
   auto comeco = this->inicio;
   auto decorrido = fim - comeco;
   constexpr auto LIMITE = milliseconds(200);

   if (decorrido > LIMITE)
      return true;
   return false;
}

void Servidor::envia_uma_entrada(Entrada& obj) const
{
   auto tubo = this->tubulacao;
   const auto QTD = MAX_SERIAL;
   auto dados = obj.serializa();

   write(tubo, dados.data(), QTD);

   // Recomeça a contagem.
   this->inicio = steady_clock::now();
   cout << "Um envio ocorreu.\n";
}

bool Servidor::entrada_pertencente(Entrada& obj)
{
   auto& fila = (*this).fila;
   int contagem = fila.size();

   while (contagem-- > 0) {
      auto itemref = fila.front();
      auto item = *itemref;

      if (obj == item) 
         { return false; }

      fila.pop();
      fila.push(itemref);
   }
   return true;
}

Servidor::Servidor(void) {
   // Começa a contar.
   this->inicio = steady_clock::now();
   // Abre o 'file descriptor' emissor de dados.
   this->tubulacao = open(PATH.c_str(), O_RDWR);

   cria_tubulacao();

   if (this->tubulacao == Failed) {
      cerr << strerror(errno) << endl;
      std::terminate();
   }
}

Servidor::~Servidor(void) {
   close(this->tubulacao);
   cout << "Cliente terminado.\n";
}

void Servidor::enviar(void) {
   auto& fila = this->fila;
   auto quantia = fila.size();

   while (quantia-- > 0) 
   {
      auto entryref = fila.front();
   
      if (this->pronto_pra_envio()) {
         auto obj = *entryref;

         this->envia_uma_entrada(obj); 
         fila.pop();
         fila.push(entryref);
      }
   }
}

bool Servidor::adiciona(Entrada* pointer) {
   auto& fila = this->fila;

   fila.push(pointer); 
   return true;
}

/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                        Cliente(Recebe as Entradas)
 *
 * O cliente delega uma fatia de tempo para ver se algo veio, então captura
 * e processa os bytes recebidos. Aqui no caso, ficou definido em 100
 * milisegundos o ritmo de tentativa de leitura.
 * == == == == == == == == == == == == == == == == == == == == == == == == */
void Cliente::insercao_controlada(Entrada& obj) 
{
/* Antes de inserir na lista interna, verifica se tal 'entrada' já não é 
 * pertencente a ela. Se este for o caso, não insere, más outra verificação
 * é realizada, está se tal é mais "adiantada" que a pertencente, então
 * troca esta pela a nova recebida. */
   size_t atual, Atual;
   int cursor{0};

   for (Entrada& entry: this->colecao) 
   {
      tie(atual, ignore, ignore) = entry.getIntAtributos();
      tie(Atual, ignore, ignore) = obj.getIntAtributos();

      if (obj == entry) {
         if (Atual > atual)
         // Atualiaza valor na array, se 'obj' passado é mais adiantado.
            this->colecao[cursor] = obj;
         return;
      }
      cursor++;
   }
   this->colecao.push_back(obj);
}

void Cliente::tenta_ler_entrada(Bytes& Out) {
/*   Tenta lê os bytes necessários para construir uma nova 'entrada'. Se não
 * for possível, por insuficiência de bytes, ou mesmo, por eles não serem
 * adequados pra formar tal objeto, provavelmente um exceção será lançada. */
   auto result = read(this->tubo, Out.data(), MAX_SERIAL);

   if (result < MAX_SERIAL) {
      throw invalid_argument("não foi possível ler todos bytes");
   }

   if (result == Failed) {
      cerr << strerror(errno) << endl;
      std::terminate();
   }
}

Cliente::Cliente(vector<Entrada>& lista): colecao(lista) {
/* O construtor deve receber uma lista, pra que se aloque todas entradas
 * externas que foram recebidas, mesmo que ela esteja incialmente vázia. 
 */
   auto caminho = PATH.c_str();

   cria_tubulacao();
   this->tubo = open(caminho, O_RDWR | O_NONBLOCK);
   this->inicio = steady_clock::now();
   this->tempo = steady_clock::now();
}

bool Cliente::permicao_de_leitura(void) {
/* Dá a confirmação de quando o possível tentar ler alguns bytes. Ele não 
 * cuida essencialmente da leitura, apenas da permição de leitura. */
   auto inicio = this->inicio;
   auto fim = steady_clock::now();
   
   if ((fim - inicio) > RITMO_DE_RECEPTACAO) 
   {
      // Reseta relógio após confirmação de condição.
      this->inicio = steady_clock::now();
      return true;
   }
   // Se o codicional não foi acionado, então o tempo decorrido ainda não
   // corresponde.
   return false;
}

bool Cliente::receber(void) {
/* Abre a porta por alguns instantes para o recebimento de dados. */
   // auto fim = steady_clock::now();
   // auto decorrido = fim - this->inicio;
   Bytes buffer;

   /*
   if (decorrido > RITMO_DE_RECEPTACAO) {
      try {
         this->tenta_ler_entrada(buffer);
         auto conversao = Entrada::deserializa(buffer);
         this->insercao_controlada(conversao);
      } catch (invalid_argument& erro) {
         // cout << "[error]" << erro.what() << endl;
      }
      this->inicio = steady_clock::now();
      // Informação operação de leitura como um sucesso.
      return true;
   }*/
   if (this->permicao_de_leitura()) 
   {
      try {
         this->tenta_ler_entrada(buffer);
         auto conversao = Entrada::deserializa(buffer);
         this->insercao_controlada(conversao);

         // Informação operação de leitura como um sucesso.
         return true;

      } catch (invalid_argument& erro) {
         // cout << "[error]" << erro.what() << endl;
         return false;
      }
   }
   // Informa que nada foi recebido.
   return false;
}

Cliente::~Cliente(void) {
/* Não apenas fecha o 'named pipe' que foi aberto para leitura; ele também
 * mostra algumas outras informações. */
   auto inicio = this->tempo;
   auto fim = steady_clock::now();
   auto decorrido = fim - inicio;
   auto seg = duration_cast<seconds>(decorrido);

   close(this->tubo);
   cout << "Tempo em aberto " << seg.count() << "seg\n";
}

#ifdef __linux__
#ifdef __unit_tests__
/* == == == == == == == == == == == == == == == == == == == == == == == == ==
 *                        Testes Unitários
 * == == == == == == == == == == == == == == == == == == == == == == == == */
#include <thread>

using namespace std::this_thread;

int main(void) {
   auto a = Entrada("Abacaxi", 25, 100);
   auto peso = 0;
   auto b = Entrada("Morango", 3, 150);
   auto server = Servidor();
   vector<Entrada> lista;
   auto client = Cliente(lista);

   // Adiciona os pointeiros mutáveis no 'servidor'.
   server.adiciona(&b);
   server.adiciona(&a);

   for (int i = 0; i < 10; i++) {
      cout << "Dados enviados.\n";

      server.enviar();
      ++a;
      ++b;
      sleep_for(1100ms);
      client.receber();
   }

   cout << "Quantia total enviada é " << peso << " bytes" << endl;
   cout << "Entradas capturadas: " << lista.size() << endl;

   for (auto& entry: lista)
      cout << entry << endl;
}
#endif
#endif

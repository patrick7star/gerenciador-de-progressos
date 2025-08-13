#include "comunicacao.hpp"
// Outros módulos deste projeto:
#include "entrada.hpp"
// Biblioteca padrão do C++:
#include <tuple>
#include <iostream>
#include <exception>
#include <system_error>
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
constexpr auto VALIDADE_ENTRADA_ESGOTADA = seconds{17};
// Apelidos:
#define Ig ignore


static void cria_tubulacao(void) {
/* Tenta criar o 'named pipe' se não existir. Se já houver, apenas uma 
 * mensagem informando o error será emitida. */
   constexpr int MODE = 0600;
   auto code = make_error_code(errc::file_exists);

   if (mkfifo(PATH.c_str(), MODE) == Failed) {
      switch (errno) {
         case EEXIST:
            throw system_error(code);
            break;
         default:
            cerr << "Erro desconhecido até o momento.\n";
            std::terminate();
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
/* Apenas permite um envio da entrada se, e somente se, passou-se um tempo
 * desde o último envio. */
   auto fim = Clock::now();
   auto comeco = this->inicio;
   auto decorrido = fim - comeco;

   if (decorrido > RITMO_DE_ENTREGA)
      return true;
   return false;
}

void Servidor::envia_uma_entrada(Entrada& obj) const
{
/* Transmissão de uma 'entrada' via tubulação, que aqui é um 'named pipe'. 
 * monitora possíveis erros durante tal empreendimento. */
   auto tubo = this->tubulacao;
   const auto QTD = MAX_SERIAL;
   auto dados = obj.serializa();

   write(tubo, dados.data(), QTD);

   // Recomeça a contagem.
   this->inicio = Clock::now();
}

bool Servidor::entrada_pertencente(Entrada& obj)
{
/* Verifica se a 'entrada' que se deseja injetar no servidor já não existe.
 * Uma função auxíliar para negar inserções. O algoritmo é, totalmente, não
 * 'thread-safe'. Isso, porque ele remove itens da fila, apesar de colocar
 * de volta, o estado dela é mudado. */
   auto& fila = (*this).fila;
   int contagem = fila.size();

   while (contagem-- > 0) {
      auto itemref = fila.front();
      auto item = *itemref;

      if (obj == item) 
         { return true; }

      fila.pop();
      fila.push(itemref);
   }
   return false;
}

static void ajusta_tubulacao(int fd) {
   // Só poderá acumular no buffer interno 4 'entradas' por vez.
   int capacidade = fcntl(fd, F_GETPIPE_SZ); 
   int num_de_paginas = capacidade / MAX_SERIAL;
   int nova_capacidade = 4 * MAX_SERIAL;

   cout << "Buffer atual: " << capacidade << " bytes" << endl <<
      "N.º de páginas: " << num_de_paginas << endl;

   num_de_paginas = fcntl(fd, F_GETPIPE_SZ) / nova_capacidade;
   // Trata erro ao não redimensionar.
   if (fcntl(fd, F_SETPIPE_SZ, nova_capacidade) == Failed) 
   {
      switch(errno) {
         case EBUSY:
            auto tipo = errc::device_or_resource_busy;
            auto codigo = make_error_code(tipo);

            throw system_error(codigo);
            break;
         // default:
         //    cerr << "[erro]" << strerror(errno) << endl;
         //    terminate();
      }
   } else {
   // Continua a visualização da alteração realizada. 
      cout << "Buffer atual: " << nova_capacidade << " bytes" << endl <<
         "N.º de páginas: " << num_de_paginas << endl;
   }
}

void Servidor::remove_entradas_expiradas(void) {
/* Retira 'entradas' que terminaram, já passado um período de tempo. */
   using time_point = system_clock::time_point;

   auto agora = system_clock::now();
   auto total = this->fila.size();
   time_point termino;
   bool pronto;
   const auto LIMITE{17s};

   while (total-- > 0) {
      auto entry = this->fila.front();   
      tie(ignore, termino, pronto) = (*entry).getTPAtributos();
      auto decorrido = agora - termino;
      auto decorrido_seg = duration_cast<seconds>(decorrido);
      
      this->fila.pop();

      if (pronto && decorrido_seg > LIMITE)
      // Se, após o término, e um tempo passado, tal 'entrada' não serve
      // mais prá transmitir.
         continue;
      else 
         this->fila.push(entry);
   }
}

Servidor::Servidor(void) {
   try 
      { cria_tubulacao(); } 
   catch(const system_error& excecao) { 
      if (excecao.code() == errc::file_exists)
          { cout << "Já tem arquivo!\n"; }
      else
         throw excecao;
   }

   // Começa a contar.
   this->inicio = Clock::now();
   // Abre o 'file descriptor' emissor de dados.
   this->tubulacao = open(PATH.c_str(), O_RDWR);

   if (this->tubulacao == Failed) {
      char* errostr = strerror(errno);
      auto msg_erro = string(errostr);
      auto tipo = errc::io_error;
      auto code = make_error_code(tipo);

      throw system_error(code, msg_erro);
   }

   try {
      ajusta_tubulacao(this->tubulacao);

   } catch(const system_error& excecao) {
      if (excecao.code() == errc::device_or_resource_busy)
         cerr << "O buffer ficará com a capacidade inicial.\n";
   }
}

Servidor::Servidor(Entrada* pointer) {
// Construtor que já vem com uma 'entrada' interna.
   try 
      { cria_tubulacao(); } 
   catch(const system_error& excecao) { 
      if (excecao.code() == errc::file_exists)
          { cout << "Já tem arquivo!\n"; }
      else
         throw excecao;
   }

   // Começa a contar.
   this->inicio = Clock::now();
   // Abre o 'file descriptor' emissor de dados.
   this->tubulacao = open(PATH.c_str(), O_RDWR);

   if (this->tubulacao == Failed) {
      char* errostr = strerror(errno);
      auto msg_erro = string(errostr);
      auto tipo = errc::io_error;
      auto code = make_error_code(tipo);

      throw system_error(code, msg_erro);
   }

   try {
      ajusta_tubulacao(this->tubulacao);

   } catch(const system_error& excecao) {
      if (excecao.code() == errc::device_or_resource_busy)
         cerr << "O buffer ficará com a capacidade inicial.\n";
   }

   if (this->adiciona(pointer))
      cout << "Servidor com 'Entrada' criada com sucesso.\n";
   else
      cout << "Apenas o servidor foi possível criar.\n";
}

Servidor::~Servidor(void) {
   close(this->tubulacao);
   cout << "Cliente terminado.\n";
}

void Servidor::enviar(void) {
/* Envia o 'status' de cada 'entrada'. Como elas estão numa fila, o processo
 * pra realizar isso é um pouco, vamos dizer, "anti-multithreading". Ele
 * remove uma 'entrada', transmite os dados dela, e coloca novamente na 
 * fila. */
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
   this->remove_entradas_expiradas();
}

bool Servidor::adiciona(Entrada* pointer) {
/* Adição de uma nova 'entrada'. Ele negará qualquer uma que já tenha na 
 * fila interna, ou já tenha sido expirada. */
   auto& fila = this->fila;

   if (!this->entrada_pertencente(*pointer)) {
      fila.push(pointer); 
      return true;
   } else
      return false;
}

bool Servidor::sem_entradas(void) const
// Informa que o servidor não tem nada a enviar.
   { return this->fila.empty(); }

int Servidor::quantidade(void) const 
// Informa 'entradas' restantes na grande fila de transmissão.
   { return this->fila.size(); }

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

      /* Igualdade significa pertencimento. Más nem sempre! Ele pode ser
       * uma igualdade, mas com valores mais atualizados, neste caso, será
       * realizado uma atualização do local na array interna. */
      if (obj == entry) {
         if (obj >= entry) {
            this->colecao[cursor] = obj;
            return;
         } else
            return;
      }
      cursor++;
   }

   /* Mesma varredura, só que na outra lista interna. A lista das 'entradas'
    * que não valem mais. */
   cursor = 0;

   /* Também não adiciona se a 'entrada' está na "seção" de expirados. */
   for (Entrada& entry: this->expiradas) {
      if (obj == entry) 
         return;
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

   try {
      cria_tubulacao();
   } catch (const system_error& excecao) {
      if (excecao.code() != errc::file_exists)
         throw excecao;
   } catch(const exception& outra)
      { throw outra; }

   this->tubo = open(caminho, O_RDWR | O_NONBLOCK);
   this->inicio = Clock::now();
   this->tempo = Clock::now();
}

bool Cliente::permicao_de_leitura(void) {
/* Dá a confirmação de quando o possível tentar ler alguns bytes. Ele não 
 * cuida essencialmente da leitura, apenas da permição de leitura. */
   auto inicio = this->inicio;
   auto fim = system_clock::now();
   
   if ((fim - inicio) > RITMO_DE_RECEPTACAO) 
   {
      // Reseta relógio após confirmação de condição.
      this->inicio = system_clock::now();
      return true;
   }
   // Se o codicional não foi acionado, então o tempo decorrido ainda não
   // corresponde.
   return false;
}

bool Cliente::receber(void) {
/* Abre a porta por alguns instantes para o recebimento de dados. */
   Bytes buffer;

   this->remocao_de_entradas_expiradas();

   if (this->permicao_de_leitura()) 
   {
      try {
         this->tenta_ler_entrada(buffer);
         auto conversao = Entrada::deserializa(buffer);
         this->insercao_controlada(conversao);

         // Informação operação de leitura como um sucesso.
         return true;

      } catch (invalid_argument& erro) {
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
   auto fim = Clock::now();
   auto decorrido = fim - inicio;
   auto seg = duration_cast<seconds>(decorrido);

   close(this->tubo);
   cout << "Tempo em aberto " << seg.count() << "seg\n";
}

static void remove_de_uma_lista_e_coloca_na_outra
  (int posicao, vector<Entrada>& In, vector<Entrada>& Out)
{
// Realiza típica remoção do item numa array, e inserção numa 'pilha'.
   int ultimo = static_cast<int>(In.size()) - 1;

   // Coloca entrada escolhida, via posição, na 'lista de expirados'.
   Out.push_back(In[posicao]);
   // Copia todos itens posteriores a ele, uma posição à frente.
   for (int n = posicao; n < ultimo; n++)
      In[n] = In[n + 1];
   // Então remove o último, que é um clone.
   In.pop_back();
}

static bool entrada_ja_esta_expirada
  (Entrada& entry, system_clock::time_point agora) 
{
/* Se tal entrada está como esgotada, e seu tempo desde de tal termino, 
 * ultrapassou um "limite" imposto, então, ele simplesmente marca tal 
 * 'entrada' como expirada. 
 */
   bool finalizada, passou_prazo;
   seconds decorrido; 
   system_clock::time_point fim;

   tie(Ig, fim, finalizada) = entry.getTPAtributos();
   decorrido = duration_cast<seconds>(agora - fim);
   passou_prazo = (decorrido > VALIDADE_ENTRADA_ESGOTADA);

   return (finalizada && passou_prazo); 
}

void Cliente::remocao_de_entradas_expiradas(void) {
   TimePoint agora = Clock::now();
   int posicao = 0;

   for (auto entry: (*this).colecao) 
   {
      if (entrada_ja_esta_expirada(entry, agora))
         remove_de_uma_lista_e_coloca_na_outra
           (posicao, (*this).colecao, (*this).expiradas);
      posicao++;
   }
}

int Cliente::quantidade(void) const
   { return static_cast<int>(this->expiradas.size()); }

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
      a += 10;
      b += 10;
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

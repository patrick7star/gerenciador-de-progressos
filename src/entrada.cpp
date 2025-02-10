// ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~
#include "entrada.hpp"
// Outros módulos do projeto:
#include "auxiliar.h"
// Bibliotecas padrões do C++:
#include <algorithm>
#include <numeric>
#include <sstream>
#include <cassert>
// Bibliotecas do Unix:
#include <ncurses.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// Puxando para fora do namespace tal funções ou objetos:
using std::string;
using std::stringstream;
using std::cout;
using std::endl;
using std::ostream;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::seconds;
using namespace auxiliar;

/* Larguras globais dos elementos redimensionados da entrada. A padronização
 * é uma boa prática para algo bem organizado. */
uint8_t VISOR_LARGURA = 32;
uint8_t BARRA_LARGURA = 41;

/* === === ===  === === === === === === === === === === === === === === ===
 *                   Implementação dos Métodos do Tipo de Dado
 *          Entrada, sejam eles operadores, constrututores, e etc...
 * === === ===  === === === === === === === === === === === === === === = */
entrada::Entrada::Entrada(
  string label, size_t a, size_t T, uint8_t vC, uint8_t bC, double tI, 
  time_point<Clock> tC
){
/* Construtor que pega todos parâmetros necessários.*/
   this->rotulo = LED(label, vC);
   this->bar = Progresso(a, T, bC);
   this->criacao = tC;
   this->taxa = tI;
}

entrada::Entrada::Entrada(string label, size_t t) {
   this->rotulo = LED(label, VISOR_LARGURA);
   this->bar = Progresso(0, t, BARRA_LARGURA);
   this->criacao = system_clock::now();
   this->taxa = 0.0;
}

entrada::Entrada::Entrada() {
   this->rotulo = LED("entrada temporário", VISOR_LARGURA);
   this->bar = Progresso(0, 1, BARRA_LARGURA);
   this->criacao = system_clock::now();
   this->taxa = 0.0;
}
// ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~

bool entrada::Entrada::operator<(Entrada& obj) {
   // Medida da instância, e então medida do argumento, respectivamente.
   auto a = this->bar.percentual();
   auto b = obj.bar.percentual();

   return a < b;
}

bool entrada::Entrada::operator>(Entrada& obj) {
   // Medida da instância, e então medida do argumento, respectivamente.
   auto a = this->bar.percentual();
   auto b = obj.bar.percentual();

   return a > b;
}

auto entrada::Entrada::operator+=(size_t quantia) -> Entrada&
   { this->bar += quantia; return *this; }

auto entrada::Entrada::operator++() -> Entrada& 
   { this->bar += 1; return *this; }
// ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~

auto entrada::Entrada::serializa() -> queue<uint8_t> {
/* A conversão de dados se dará na seguinte forma: Primeiro o texto, rótulo,
 * nome,... como você quiser chamar-lo, entretanto, antes de injetar isso
 * como bytes, é preciso armazenar de quantos bytes estamos realmente
 * falando. O próximo é os dois valores que definem o progresso, o atual
 * valor, e a meta, ambos que são inteiros positivos com tamanho da máquina.
 * A taxa do aumento percentual também é importante, não a mais, poderia
 * sim, ser computada neste atual processo, porém se já vem processado, 
 * porque fazer isso. O último ponto traduzido em bytes é o 'timestamp' que
 * criou o "progresso", ou seja, quanto tempo está executando isso, ele é
 * um inteiro com sinal, mais tamanho de máquina.
 *
 *   Seguir tal ordem é importante para futuramente a deserialização e 
 * decodificação de tal continuos array de bytes seja facilitada, ela 
 * seguirá a mesma sequência de processamento desta função aqui. */
   const int sz = sizeof(size_t);
   queue<uint8_t> Out;
   /* Selo de tempo quando criado tal coisa. */
   time_t t = system_clock::to_time_t(this->criacao);
   
   auto Out_a = this->rotulo.serializa();
   auto Out_b = serializa_inteiro((size_t)t); 
   auto Out_c = serializa_decimal(this->taxa);
   auto Out_d = this->bar.serializa();
   /* Contabilizando o total de bytes do conjunto acima, inclusive deste
    * aqui. A letra bem distante das demais indica que não seguirá a ordem
    * de concatenação das demais. Ele no quesito tem que ser o primeiro
    * a ser concatenado, pois indica o total de bytes a extrair de uma
    * fila contendo a aglomeração de todos bytes. */
   auto Out_z = serializa_inteiro(
      Out_a.size() + Out_b.size() + 
      Out_c.size() + Out_d.size() + sz
   );

   anexa_fila(std::move(Out_z), Out);
   anexa_fila(std::move(Out_a), Out);
   anexa_fila(std::move(Out_b), Out);
   anexa_fila(std::move(Out_c), Out);
   anexa_fila(std::move(Out_d), Out);

   return Out;
}

auto entrada::Entrada::deserializa(queue<uint8_t>& In) -> Entrada {
/* O esquema é seguir o procedimento acima. Como o resultado é uma fila 
 * de bytes, então os primeiros serializados, serão obviamente os primeiros
 * a serem deserializado. O primeiro aqui é o tanto de bytes que a Entrada
 * ocupa, por ser uma geração variavel foi necessário colocar tal tanto
 * de bytes codificados no começo da "linguiça de bytes". Decodificado eles
 * você pode extrair o total de bytes necessários para decodificar tal 
 * tipo. */
   auto bytes = extrai_n_bytes(In, sizeof(size_t));
   auto total = deserializa_inteiro(bytes);

   assert (In.size() >= total);
   // Restantes do bytes a extrair, já faz tudo de uma vez só.
   auto r = extrai_n_bytes(bytes, total);

   auto rotulo  = LED::deserializa(r);
   auto criacao = deserializa_inteiro(r);
   auto taxa    = deserializa_decimal(r);
   auto bar     = Progresso::deserializa(r);

   // Tudo deve ter sido devidamente extraído.
   assert (r.empty());

   return Entrada(rotulo.texto, bar.atual, bar.total, rotulo.comprimento,
         bar.comprimento, taxa, system_clock::from_time_t(criacao));
}

/* === === ===  === === === === === === === === === === === === === === ===
 *                   Sobrecarga de alguns métodos e novos
 *                         para o tipo EntradaPipe
 * === === ===  === === === === === === === === === === === === === === = */
static entrada::path SEP_PTH = entrada::path("#");

auto entrada::EntradaPipe::cria_named_pipe_automatico() -> void {
   mode_t nivel_de_permissao = 0664;
   path caminho = this->canal;
   int result = mkfifo(caminho.c_str(), nivel_de_permissao);
   const char* nome = caminho.filename().c_str();

   if (result == -1) {
      switch(errno) {
         case EACCES:
            cout << "Algum diretório no caminho não permite pesquisa.\n";
            break;
         case EEXIST:
            cout << "O caminho já existe.\n";
            break;
         case EROFS:
            cout << "Aquivo com apenas permissão de leitura.\n";
            break;
         default:
            cout << "Qualquer outro erro na criação do 'named pipe' "
               << nome << endl;
      }
   }
}

auto entrada::EntradaPipe::abre_named_pipe_automatico() -> void {
   const char* caminho = this->canal.c_str();
   auto nome = this->canal.filename();
   this->fd = open(this->canal.c_str(), O_RDWR);

   if (this->fd == -1) {
      switch(errno) {
         case EACCES:
            cout << "Acesso negado em alguma parte do caminho.\n";
            break;

         case EEXIST:
            cout << "Caminho " << caminho << " já existe.\n";
            break;

         default:
            cout << "Outro erro na abertura do np " << caminho << endl;
      }
   }
}

entrada::EntradaPipe::EntradaPipe(
  pid_t id, path cH, int fd, string l, size_t a, size_t T, uint8_t vC, 
  uint8_t bC, double tI, time_point<Clock> tC
): Entrada(l, a, T, vC, bC, tI, tC)
{
   this->ID = id;
   this->canal = cH;
   this->fd = fd;

   this->cria_named_pipe_automatico();
   this->abre_named_pipe_automatico();
}

entrada::EntradaPipe::EntradaPipe(path cH, string l, size_t t): Entrada(l, t)
{
   pid_t identificador = getpid();
   string id_str = std::to_string(identificador);

   this->ID = identificador;
   this->canal += cH;
   this->canal += SEP_PTH; 
   this->canal += path(id_str);

   this->cria_named_pipe_automatico();
   this->abre_named_pipe_automatico();
}

entrada::EntradaPipe::EntradaPipe(): Entrada() {
   pid_t identificador = getpid();
   string id_str = std::to_string(identificador);
   path canal = path(string("entrada_pipe"));

   this->ID = identificador;
   this->canal += canal;
   this->canal += SEP_PTH; 
   this->canal += path(id_str);

   this->cria_named_pipe_automatico();
   this->abre_named_pipe_automatico();
}

entrada::EntradaPipe::~EntradaPipe() {
   const char* caminho = this->canal.c_str();

   if (close(this->fd) == -1) {
      switch (errno) {
         case EBADF:
            cout << "O 'fd' é inválido.\n";
            break;
         case EINTR:
            cout << "A chamada 'close' foi interrompida por um sinal.\n";
            break;
         case EIO:
            cout << "Erro no I/O durante o fechamento.\n";
            break;
         default:
            cout << "Outro erro incomum durante a chamada do 'close'.\n";
      }
   }

   if (unlink(caminho) == -1) {
      switch (errno) {
         case EROFS:
            cout << "Arquivo com apenas permissão de escrita.\n";
            break;
         case EISDIR:
            cout << "É na verdade um diretório. Não um 'named pipe'.\n";
            break;
         case ENOTDIR:
            cout << "Caminho relativo não apontando certo 'named pipe'.\n";
            break;
         default:
            cout << "Erro ao tentar excluir o 'named pipe'.\n";
      }
   }
}
// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- -

bool entrada::EntradaPipe::operator==(EntradaPipe& obj) {
/* O mesmo 'id' referente ao processo, também o mesmo 'file descripto', assim
 * como o caminho. Compartilhando todos estes valores, podemos dizer que 
 * tal tipo de 'entrada' é igual. Isso não parece intuítivo baseado na sua
 * superclass, porém tal igualdade aqui é para descartar repetições. Isto
 * que numa malha de multiprocessamento acontecerá bastante. */
   return (obj.ID == this->ID) && (obj.fd == this->fd) 
            && (obj.canal == this->canal);
}
ostream& operator<<(ostream& output, entrada::EntradaPipe& obj) {
   output.width(4);
   output.precision(2);
   output << "[Entrada Pipe] " << obj.bar.atual << " de " << 
      obj.bar.total << "\t~" << obj.bar.percentual() << '%';

   return output;
}

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- -
auto entrada::EntradaPipe::serializa() -> queue<uint8_t> {
/* Quase o mesmo que o da superclasse original, porém com os campos extras
 * que este tem. E claro, todos eles são inseridos na ordem que foram 
 * codificados, no final da seŕie de bytes que o primeiro já realiza, método
 * do antigo. */
   queue<uint8_t> Out_d, Out_a, Out_b, Out_c, Out_sz, Out;
   std::string caminho{this->canal.c_str()};
   size_t fd = this->fd, id = this->ID, size;
   
   Out_a = Entrada::serializa();
   size = deserializa_inteiro(Out_a);

   Out_b = serializa_inteiro(id);
   Out_c = serializa_string(caminho);
   Out_d = serializa_inteiro(fd);
   Out_sz = serializa_inteiro(
      size + Out_a.size() + Out_b.size() + 
      Out_c.size() + Out_d.size()
   );

   /* Primeiro a quantia de bytes do demais, depois o resto. Apenas segue-se
    * a ordem alfabética que já foi ajeitado desta maneira. */
   anexa_fila(std::move(Out_sz), Out);
   anexa_fila(std::move(Out_a), Out);
   anexa_fila(std::move(Out_b), Out);
   anexa_fila(std::move(Out_c), Out);
   anexa_fila(std::move(Out_d), Out);

   return Out;
}

auto entrada::EntradaPipe::deserializa(queue<uint8_t>& In) -> EntradaPipe { 
/* Deserializa e codifica os primeiros bytes, estes que codificam os demais
 * bytes. Extrai tal quantia, e começa a deserializar e atribuir o valor
 * aos determinados campos. */
   auto total = deserializa_inteiro(In);
   assert (In.size() >= total);
   // Restantes do bytes a extrair, já faz tudo de uma vez só.
   auto r = extrai_n_bytes(In, total);

   // Dados da Entrada original:
   auto rotulo  = LED::deserializa(r);
   auto criacao = deserializa_inteiro(r);
   auto taxa    = deserializa_decimal(r);
   auto bar     = Progresso::deserializa(r);
   // Dados da EntradaPipe:
   auto pid     = deserializa_inteiro(r);
   auto canal   = deserializa_string(r);
   auto fd      = deserializa_inteiro(r);

   // Tudo deve ter sido devidamente extraído.
   assert (r.empty());
   return EntradaPipe(
      pid, canal, fd, rotulo.texto, bar.atual, bar.total, 
      rotulo.comprimento, bar.comprimento, taxa, 
      system_clock::from_time_t(criacao)
   );
}

static uint8_t* fila_to_array_de_bytes(queue<uint8_t>&& fila)
{
   int size = fila.size();
   uint8_t* array_ = new uint8_t[size];
   int cursor = 0;

   while (!fila.empty()) {
      array_[cursor++] = fila.front();
      fila.pop();
   }
   return array_;
}

void entrada::EntradaPipe::atualiza_externo() {
   auto qb = this->serializa();
   auto quantia = qb.size();
   auto array_ = fila_to_array_de_bytes(std::move(qb));

   write(this->fd, array_, quantia);
   delete[] array_;
}

#ifdef __unit_tests__
/* === === ===  === === === === === === === === === === === === === === ===
 *                      Testes Unitários
 * === === ===  === === === === === === === === === === === === === === = */
#include "teste.h"
#include <thread>

using entrada::EntradaPipe;
using entrada::string;

int main(void) {
   auto texto = string("Isso é um teste com um pipe teórico");
   auto X = EntradaPipe(texto, 800);
   auto duracao = std::chrono::seconds(2);

   for (int i = 0; i < 12; i++) {
      cout << X << endl;
      X += (5 + i) * i;   
      std::this_thread::sleep_for(duracao);
   }
}
#endif

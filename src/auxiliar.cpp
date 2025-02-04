#include "auxiliar.h"
#include <iostream>
#include <cassert>

using std::cout;
using std::endl;
using auxiliar::queue;

void auxiliar::print_queue(queue<uint8_t>& X) 
{
   if (X.empty())
      { cout << 0 << " itens | []" << endl; return; }

	auto T = X.size();
   // Registrando quantia inicial.
   auto qI = T;
   // Também as suas pontas, nada pode ficar desorganizados no final.
   auto frente = X.front();
   auto calda = X.back();

	cout << (int)T << " itens | "<< '[';

   while (T-- > 0) 
   {
      auto removido = X.front();
      // Deixa-o imprimível para a saída padrão do sistema.
      int fmt = (int)removido;

      /* Retira elemento copiado, e em seguida o coloca no fim da fila
       * novamente. */
      X.pop(); X.push(removido);

		cout << fmt << ',' << ' ';
	}

	cout << "\b\b" << ']' << endl;
   // Tem que terminar com a mesma quantia que iniciou.
   assert (qI == X.size());
   assert (calda == X.back());
   assert (frente == X.front());
}

template<typename T> void auxiliar::print_vector(vector<T>& l) 
{
   const char VIR = ',';
   const char BR = ' ';
   const char RC = '\b';
   cout << '[';
   for (auto& e: l)
      cout << (int)e << VIR << BR;
   cout << RC << RC << ']' << endl;
}

void auxiliar::print_array_u8(uint8_t* array, int t) {
	cout << '(' << t << ')' << '[';

	for (int k = 1; k <= t; k++)
		cout << (int)array[k - 1] << ", ";

	cout << "\b\b]\n";
}

queue<uint8_t> auxiliar::extrai_n_bytes(queue<uint8_t>& In, size_t n) {
/* Pega uma fila com bytes, e extrai 'n' bytes da frente dela. É como se
 * ela fosse partida em dois à partir de uma posição 'n - 1' dela. */
	assert(In.size() >= n);
	
	queue<uint8_t> Out;
	size_t N = In.size();

   while (Out.size() < n)
      { Out.push(In.front()); In.pop(); }

	// Verificação antes do retorno...
	assert (Out.size() == n);
	assert (In.size() + n == N);

	return Out;
}

void auxiliar::queue_to_array_u8(queue<uint8_t>& In, uint8_t* Out) {
/* Pega uma fila com todos bytes que podem ser inseridos na array, e os 
 * insere. É importante que a array passada tem o tamanho de bytes que 
 * a fila insere. */
	size_t n = 0;

	do {
		Out[n++] = In.front();
		In.pop();

	} while(not In.empty());
}

auto auxiliar::espia_tamanho(queue<uint8_t>& f) -> size_t {
/* Se tiver o mínimo de bytes necessário, pega os primeiros bytes, o mesmo
 * equivalente do de máquina, então forma um inteiro e retorna. Apesar de 
 * haver remoção/inserção na fila, ela termina com os elementos em mesma
 * quantia e ordem que começou. Entretanto, obviamente tal função não 
 * garante thread-safety, porque há mudança interna no argumento passado. */
	constexpr size_t Sz = sizeof(size_t);
	size_t T = f.size(), p = 0;
	uint8_t buffer[Sz];

   // A fila tem que ter no mínimo 8 bytes.
	assert(T >= Sz);

   /* Ciclo de processamento é o seguinte: copia o primeiro valor da fila,
    * retira dele, e c no final coloca o valor copiado de volta no fim
    * dela. */
   while (p < T) {
      auto e = f.front();

      /* Apenas adiciona na array os 'Sz' primeiros. */
      if (p < Sz) 
         buffer[p] = e;

      f.pop();
      f.push(e);
      p++;
   }
	// Tem que terminar com o mesmo tamanho que começou.
	assert(f.size() == T);
	return from_bytes_to_sizet(buffer);
}

auto auxiliar::queue_to_struct_bytes(queue<uint8_t>& In) -> struct Bytes
{
/* Não é possível determinar um mínimo a ser extrair inicialmente, porque
 * a string é dinâmica, entretanto, é possível computar tal limite que foi
 * compactado na fila de bytes, então determinar se a fila passada tem os
 * bytes necessários para extrair uma 'struct Bytes'. */
   size_t n = espia_tamanho(In), sz = sizeof(size_t);
   size_t N = n + sz;
   struct Bytes Out;

   Out.total = N;
   Out.bytes = (uint8_t*)malloc(N);

   auto seq = extrai_n_bytes(In, N);
   assert (Out.bytes != nullptr);
   queue_to_array_u8(seq, Out.bytes);

	return Out;
}

auto auxiliar::serializa_string(string& s) -> queue<uint8_t> {
/* Converte um objeto string(do C++) numa array dinâmica de bytes, logo
 * a retorna. */
	char* ptr = (char*)s.c_str();
	struct Bytes saida = string_to_bytes(ptr);
	size_t n = saida.total;
	queue<uint8_t> bytes;

	for(size_t i = 0; i < n; i++)
		bytes.push(saida.bytes[i]);
	return bytes;
}

auto auxiliar::deserializa_string(queue<uint8_t>& bytes) -> string {
	struct Bytes entrada;
	char* saida;
	string resultado;
   
	// Ciclo de processamento ...
	entrada = queue_to_struct_bytes(bytes);
	saida = from_bytes_to_string(&entrada);
	// Liberando array de bytes alocada dentro da estrutura 'Bytes'.
	resultado = string(saida);
	/* Liberando array de caractéres formadada durante a conversão de 
	 * 'struct Bytes' para pointeiro de 'char'. */
	free_bytes(&entrada);
	free(saida);

	return resultado;
}

auto auxiliar::serializa_inteiro(size_t n) -> queue<uint8_t> {
/* Pego o maior inteiro de máquina, então extrai seus bytes para uma array
 * dinâmica. */
	queue<uint8_t> bytes;
	int N = sizeof(size_t);
	uint8_t buffer[N];

	// Convertendo...
	sizet_to_bytes(n, buffer);
	// Passando da array estática para a dinâmica.
	for (int i = 0; i < N; i++)
		bytes.push(buffer[i]);

   assert (bytes.size() == N);
	return bytes;
}

auto auxiliar::deserializa_inteiro(queue<uint8_t>& In) -> size_t {
/* Pega uma fila com vários bytes, consome o mínimo para gerar o inteiro
 * positivo de máquina, então cospe tal valor processsado. A fila terminará
 * como menos bytes do que entrou, sendo está diferença o tanto do tipo
 * 'size_t'. */
   assert (In.size() >= sizeof(size_t));

	const size_t sz = sizeof(size_t);
   size_t qtd = In.size();
	uint8_t buffer[sz];
   /* Retira a determinada quantia de bytes para deserializar o inteiro
    * de máquina. */
   auto parte = extrai_n_bytes(In, sz);
	queue_to_array_u8(parte, buffer);

	return from_bytes_to_sizet(buffer);
}

void auxiliar::anexa_fila(queue<uint8_t>&& In, queue<uint8_t>& Out) 
{
/* Pega a 'propriedade' de uma fila, extrai todos seus itens na sua ordem
 * natural, então insere na outra dada. 
 */
   size_t n = In.size(), m = Out.size();

	while (not In.empty()) {
      auto data = In.front();

		In.pop();
		Out.push(data);
	}

   // Confirmando que tudo foi passado.
   assert (Out.size() == m + n);
}

auto auxiliar::array_to_queue(uint8_t* In, size_t n) -> queue<uint8_t> {
   queue<uint8_t> Out; 

   for (size_t i = 1; i <= n; i++)
      Out.push(In[n - 1]);

   return Out;
}

auto auxiliar::serializa_decimal(double In) -> queue<uint8_t> {
   uint8_t buffer[sizeof(double)];
   queue<uint8_t> Out, qB;

   double_to_bytes(In, buffer);
   qB = array_to_queue(buffer, 8);
   auxiliar::anexa_fila(std::move(qB), Out);

   return Out;
}

auto auxiliar::deserializa_decimal(queue<uint8_t> In) -> double
{
   const int sz = sizeof(double);
   assert (In.size() >= sz);

   uint8_t buffer[sz];
   // Fila dos bytes necessários para conversão em double.
   auto bQ = extrai_n_bytes(In, sz);

   queue_to_array_u8(bQ, buffer);
   /* Converte a array com bytes, e já retorna em seguida. */
   return from_bytes_to_double(buffer);
}


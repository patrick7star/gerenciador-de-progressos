#pragma once
#include <vector>
#include <queue>
#include <string>
#include <cstdint>
// Bibliotecas externas:
extern "C" {
  #include "conversao.h"

  struct Bytes string_to_bytes (char*);
  char*  from_bytes_to_string  (struct Bytes*);

  void   sizet_to_bytes        (size_t, uint8_t*);
  size_t from_bytes_to_sizet   (uint8_t*);

  void   double_to_bytes       (double, uint8_t*); 
  double from_bytes_to_double  (uint8_t*);
}

namespace auxiliar {
  using std::vector;
  using std::queue;
  using std::size_t;
  using std::string;

  template<typename T> void print_vector(vector<T>&);
  void print_queue(queue<uint8_t>& X);
  void print_array_u8(uint8_t* array, int t); 

  // queue<uint8_t> extrai_n_bytes(queue<uint8_t>& In, size_t n); 
  auto extrai_n_bytes(queue<uint8_t>& In, size_t n) -> queue<uint8_t>; 

  void queue_to_array_u8(queue<uint8_t>& In, uint8_t* Out); 
  auto array_to_queue(uint8_t* In, size_t n) -> queue<uint8_t>;

  auto espia_tamanho(queue<uint8_t>& f) -> size_t;
  auto queue_to_struct_bytes(queue<uint8_t>& In) -> struct Bytes;

  auto serializa_string(string& s) -> queue<uint8_t>; 
  auto deserializa_string(queue<uint8_t>& bytes) -> string; 

  auto serializa_inteiro(size_t n) -> queue<uint8_t>; 
  auto deserializa_inteiro(queue<uint8_t>& In) -> size_t; 

  void anexa_fila(queue<uint8_t>&& In, queue<uint8_t>& Out);
  auto array_to_queue(uint8_t* In, size_t n) -> queue<uint8_t>; 

}

namespace auxiliar {
  auto serializa_decimal(double In) -> queue<uint8_t>; 
  auto deserializa_decimal(queue<uint8_t> In) -> double;
}

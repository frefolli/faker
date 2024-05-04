#ifndef SIGMOD_MEMORY_HH
#define SIGMOD_MEMORY_HH

#include <cstring>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <sigmod/debug.hh>

template<typename T>
T* smalloc(uint32_t length = 1, std::string help = "") {
  const uint32_t n_of_bytes = sizeof(T) * length;
  T* ptr = new T[length];
  if (nullptr == ptr) {
    std::string error_message = "no more memory available, was trying to allocate " + BytesToString(n_of_bytes);
    if (help.size() != 0)
      error_message += " for " + help;
    throw std::runtime_error(error_message);
  }
  return ptr;
}

template<typename T>
void sfree(T* ptr, std::string help = "") {
  if (nullptr != ptr) {
    delete[] ptr;
  } else {
    std::string error_message = "sfree called on nullptr";
    if (help.size() != 0)
      error_message += " for " + help;
    throw std::runtime_error(error_message);
  }
}
#endif

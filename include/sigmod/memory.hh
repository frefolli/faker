#ifndef MEMORY_HH
#define MEMORY_HH
#include <sigmod/debug.hh>
#include <sigmod/config.hh>

template<typename T>
T* smalloc(uint32_t length = 1, std::string help = "") {
  const uint32_t n_of_bytes = sizeof(T) * length;
  T* ptr = (T*) malloc(n_of_bytes);
  if (nullptr == ptr) {
    std::string error_message = "no more memory available, was trying to allocate " + BytesToString(n_of_bytes);
    if (help.size() != 0)
      error_message += " for " + help;
    throw std::runtime_error(error_message);
  }
  return ptr;
}
#endif

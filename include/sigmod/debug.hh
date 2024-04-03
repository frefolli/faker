#ifndef DEBUG_HH
#define DEBUG_HH

#include <iostream>

inline void Debug(std::string s) {
    std::cout << "# DEBUG | " << s << std::endl;
}

inline void Panic(std::string s) {
    std::cout << "# PANIC | " << s << std::endl;
    exit(1);
}

void LogTime(std::string s);

#endif

#ifndef SIGMOD_DEBUG_HH
#define SIGMOD_DEBUG_HH

#include <iostream>
#include <string>

inline void Debug(const std::string s) {
    std::cout << "# DEBUG | " << s << std::endl;
}

inline void Panic(const std::string s) {
    std::cout << "# PANIC | " << s << std::endl;
    exit(1);
}

void LogTime(const std::string s);

extern long long SIGMOD_MEMORY_TRACKER;
extern long long SIGMOD_DISTANCE_COMPUTATIONS;

std::string BytesToString(long double bytes);
std::string LengthToString(long double length);
void LogMemory(const std::string s);

#endif

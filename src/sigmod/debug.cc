#include <sigmod/debug.hh>
#include <chrono>

auto SIGMOD_LOG_TIME = std::chrono::high_resolution_clock::now();

void LogTime(std::string s) {
    auto now = std::chrono::high_resolution_clock::now();
    std::cout << "# TIME | " << s << " | el. "
        << std::chrono::duration_cast<std::chrono::milliseconds>(now - SIGMOD_LOG_TIME).count()
        << " ms" << std::endl;
    SIGMOD_LOG_TIME = now;
}

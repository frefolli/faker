#include <sigmod/debug.hh>
#include <chrono>
#include <cmath>

auto SIGMOD_LOG_TIME = std::chrono::high_resolution_clock::now();

void LogTime(const std::string s) {
    auto now = std::chrono::high_resolution_clock::now();
    std::cout << "# TIME | " << s << " | el. "
        << std::chrono::duration_cast<std::chrono::milliseconds>(now - SIGMOD_LOG_TIME).count()
        << " ms" << std::endl;
    SIGMOD_LOG_TIME = now;
}

inline std::string sround(long double value, uint32_t precision = 3) {
    std::string rep = std::to_string(value);
    for (uint32_t i = 0; i < rep.size(); i++) {
        if (rep.at(i) == '.') {
            uint32_t len = std::min((uint32_t)rep.size(), i + precision + (uint32_t)1);
            return rep.substr(0, len);
        }
    }
    return rep;
}

std::string BytesToString(long double bytes) {
    std::string rep = "";
    static const long double KILO = 1024;
    static const long double MEGA = KILO*1024;
    static const long double GIGA = MEGA*1024;
    static const long double TERA = GIGA*1024;
    if (bytes > TERA) {
        rep = sround(bytes/TERA) + " TB";
    } else if (bytes > GIGA) {
        rep = sround(bytes/GIGA) + " GB";
    } else if (bytes > MEGA) {
        rep = sround(bytes/MEGA) + " MB";
    } else if (bytes > KILO) {
        rep = sround(bytes/KILO) + " KB";
    } else {
        rep = std::to_string(bytes) + " B";
    }
    return rep;
}

std::string LengthToString(long double length) {
    std::string rep = "";
    static const long double KILO = 1024;
    static const long double MEGA = KILO*1024;
    static const long double GIGA = MEGA*1024;
    static const long double TERA = GIGA*1024;
    if (length > TERA) {
        rep = sround(length/TERA) + " T";
    } else if (length > GIGA) {
        rep = sround(length/GIGA) + " G";
    } else if (length > MEGA) {
        rep = sround(length/MEGA) + " M";
    } else if (length > KILO) {
        rep = sround(length/KILO) + " K";
    } else {
        rep = std::to_string((long long) length);
    }
    return rep;
}

long long SIGMOD_MEMORY_TRACKER = 0;
long long SIGMOD_DISTANCE_COMPUTATIONS = 0;

void LogMemory(const std::string s) {
    std::cout << "# MEMORY | " << s << " | Allocated " << BytesToString(SIGMOD_MEMORY_TRACKER) << std::endl;
    SIGMOD_MEMORY_TRACKER = 0;
}

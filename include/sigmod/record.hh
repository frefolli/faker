#ifndef RECORD_HH
#define RECORD_HH

#include <ostream>
#include <sigmod/config.hh>

struct Record {
    float32_t C;
    float32_t T;
    float32_t fields[vector_num_dimension];
};

std::ostream& operator<<(std::ostream& out, const Record& record);

#endif

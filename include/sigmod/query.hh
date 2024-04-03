#ifndef QUERY_HH
#define QUERY_HH

#include <ostream>
#include <sigmod/config.hh>

struct Query {
    float32_t query_type;
    float32_t v;
    float32_t l;
    float32_t r;
    float32_t fields[vector_num_dimension];
};

enum query_t {
    NORMAL = 0,
    BY_C = 1,
    BY_T = 2,
    BY_C_AND_T = 3
};

std::ostream& operator<<(std::ostream& out, const Query& query);

#endif

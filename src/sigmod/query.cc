#include <sigmod/query.hh>
#include <iostream>

std::ostream& operator<<(std::ostream& out, const Query& query) {
    out << query.query_type << "|" << query.v << "|" << query.l << "|" << query.r;
    for (uint32_t i = 0; i < vector_num_dimension; i++)
        out << "|" << query.fields[i];
    return out;
}

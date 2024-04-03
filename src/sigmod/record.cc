#include <sigmod/record.hh>
#include <iostream>

std::ostream& operator<<(std::ostream& out, const Record& record) {
    out << record.C << "|" << record.T;
    for (uint32_t i = 0; i < vector_num_dimension; i++)
        out << "|" << record.fields[i];
    return out;
}

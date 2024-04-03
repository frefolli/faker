#include "sigmod/config.hh"
#include "sigmod/database.hh"
#include "sigmod/random.hh"
#include <sigmod.hh>
#include <iostream>
#include <ctime>

const uint32_t CATEGORIES = 1000;
const float32_t X = 5;
void RandomizeRecord(Record& record) {
  record.C = RandomUINT32T(0, CATEGORIES-1);
  record.T = RandomFLOAT32T(0, 1);
  for (uint32_t i = 0; i < vector_num_dimension; i++) {
    record.fields[i] = RandomFLOAT32T(-X, X);
  }
}

void RandomizeDatabase(Database& database) {
  for (uint32_t i = 0; i < database.length; i++) {
    RandomizeRecord(database.records[i]);
  }
}

Database NewDatabase(uint32_t length) {
  return {
    .length = length,
    .records = (Record*) malloc (sizeof(Record) * length)
  };
}

int main(int argc, char** args) {
    Database db = NewDatabase(1000000);
    RandomizeDatabase(db);
    FreeDatabase(db);
  
    std::srand(std::time(0));
    std::cout << RandomFLOAT32T(0, 1) << std::endl;
    std::cout << RandomFLOAT32T(0, 1) << std::endl;
    std::cout << RandomFLOAT32T(0, 1) << std::endl;
    std::cout << RandomFLOAT32T(0, 1) << std::endl;
    std::cout << RandomFLOAT32T(0, 1) << std::endl;
    std::cout << RandomFLOAT32T(0, 1) << std::endl;
}

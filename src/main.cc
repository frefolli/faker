#include "sigmod/config.hh"
#include "sigmod/database.hh"
#include "sigmod/random.hh"
#include <sigmod.hh>
#include <iostream>
#include <ctime>

const uint32_t CATEGORIES = 1000;
const float32_t X = 5;

void RandomizeRecord(Record& record) {
    record.C = RandomUINT32T(0, CATEGORIES);
    record.T = RandomFLOAT32T(0, 1);
    for (uint32_t i = 0; i < vector_num_dimension; i++) {
        record.fields[i] = RandomFLOAT32T(-X, X);
    }
}

void CraftQuery(Query& query, const Record& record) {
    const uint32_t query_type = RandomUINT32T(0, 4);

    query.query_type = query_type;
    switch (query_type) {
        case NORMAL: {
            query.v = -1;
            query.l = -1;
            query.r = -1;
        }; break;
        case BY_C: {
            query.v = record.C;
            query.l = -1;
            query.r = -1;
        }; break;
        case BY_T: {
            query.v = -1;
            query.l = RandomFLOAT32T(0, record.T);
            query.r = RandomFLOAT32T(record.T, 1);
        }; break;
        case BY_C_AND_T: {
            query.v = record.C;
            query.l = RandomFLOAT32T(0, record.T);
            query.r = RandomFLOAT32T(record.T, 1);
        }; break;
    }

    for (uint32_t i = 0; i < vector_num_dimension; i++) {
        query.fields[i] = record.fields[i];
    }
}

void RandomizeDatabase(Database& database) {
    for (uint32_t i = 0; i < database.length; i++) {
        RandomizeRecord(database.records[i]);
    }
}

void CraftQuerySet(QuerySet& queryset, const Database& database) {
    for (uint32_t i = 0; i < queryset.length; i++) {
        CraftQuery(queryset.queries[i], database.records[i]);
    }
}

Database NewDatabase(uint32_t length) {
    return {
        .length = length,
        .records = (Record*) malloc (sizeof(Record) * length)
    };
}

QuerySet NewQuerySet(uint32_t length) {
    return {
        .length = length,
        .queries = (Query*) malloc (sizeof(Query) * length)
    };
}

int main(int argc, char** args) {
    std::srand(std::time(0));

    Database db = NewDatabase(1000000);
    RandomizeDatabase(db);
    WriteDatabase(db, "faker-data.bin");

    QuerySet qs = NewQuerySet(1000);
    CraftQuerySet(qs, db);
    WriteQuerySet(qs, "faker-queries.bin");

    FreeDatabase(db);
    FreeQuerySet(qs);
}

#include <sigmod/query_set.hh>
#include <cstdio>
#include <iostream>
#include <map>
#include <algorithm>

QuerySet ReadQuerySet(std::string input_path) {
    FILE* dbfile = fopen(input_path.c_str(), "rb");
    
    uint32_t db_length;
    fread(&db_length, sizeof(uint32_t), 1, dbfile);

    Query* queries = (Query*) std::malloc(sizeof(Query) * db_length);
    Query* queries_entry_point = queries;
    uint32_t queries_to_read = db_length;
    while(queries_to_read > 0) {
        uint32_t this_batch = batch_size;
        if (this_batch > queries_to_read) {
            this_batch = queries_to_read;
        }
        fread(queries_entry_point, sizeof(Query), this_batch, dbfile);
        queries_to_read -= this_batch;
        queries_entry_point += this_batch;
    }
    fclose(dbfile);

    return {
        .length = db_length,
        .queries = queries
    };
}

void WriteQuerySet(QuerySet& query_set, std::string output_path) {
    FILE* output = fopen(output_path.c_str(), "wb");

    fwrite(&query_set.length, sizeof(uint32_t), 1, output);
    Query* query_set_entry_point = query_set.queries;
    uint32_t query_set_to_write = query_set.length;
    while(query_set_to_write > 0) {
        uint32_t this_batch = batch_size;
        if (this_batch > query_set_to_write) {
            this_batch = query_set_to_write;
        }
        fwrite(query_set_entry_point, sizeof(Query), this_batch, output);
        query_set_to_write -= this_batch;
        query_set_entry_point += this_batch;
    }

    fclose(output);
}

void FreeQuerySet(QuerySet& queryset) {
    if (queryset.queries == nullptr)
        return;
    free(queryset.queries);
    queryset.queries = nullptr;
    queryset.length = 0;
}

#include <sigmod/database.hh>
#include <cstdio>
#include <algorithm>
#include <iostream>

Database ReadDatabase(std::string input_path) {
    FILE* dbfile = fopen(input_path.c_str(), "rb");
    
    uint32_t db_length;
    fread(&db_length, sizeof(uint32_t), 1, dbfile);

    Record* records = (Record*) std::malloc(sizeof(Record) * db_length);
    Record* records_entry_point = records;
    uint32_t records_to_read = db_length;
    while(records_to_read > 0) {
        uint32_t this_batch = batch_size;
        if (this_batch > records_to_read) {
            this_batch = records_to_read;
        }
        fread(records_entry_point, sizeof(Record), this_batch, dbfile);
        records_to_read -= this_batch;
        records_entry_point += this_batch;
    }
    fclose(dbfile);

    return {
        .length = db_length,
        .records = records
    };
}

void WriteDatabase(const Database& database, std::string input_path) {
    FILE* dbfile = fopen(input_path.c_str(), "wb");
    
    uint32_t db_length = database.length;
    fwrite(&db_length, sizeof(uint32_t), 1, dbfile);

    Record* records_entry_point = database.records;
    uint32_t records_to_write = db_length;
    while(records_to_write > 0) {
        uint32_t this_batch = batch_size;
        if (this_batch > records_to_write) {
            this_batch = records_to_write;
        }
        fwrite(records_entry_point, sizeof(Record), this_batch, dbfile);
        records_to_write -= this_batch;
        records_entry_point += this_batch;
    }
    fclose(dbfile);
}

void FreeDatabase(Database& database) {
    if (database.records == nullptr)
        return;
    free(database.records);
    database.records = nullptr;
    database.length = 0;
}

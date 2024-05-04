#include <sigmod/config.hh>
#include <sigmod/query_set.hh>
#include <sigmod/database.hh>
#include <sigmod/memory.hh>
#include <sigmod/scoreboard.hh>
#include <sigmod/flags.hh>
#include <omp.h>
#include <algorithm>
#include <cassert>

struct Index {
  uint32_t* indices;
  uint32_t length;

  static Index* New(Database& db) {
    uint32_t* indices = smalloc<uint32_t>(db.length);
    for (uint32_t i = 0; i < db.length; i++) {
      indices[i] = i;
    }

    return new Index {
      .indices = indices,
      .length = db.length
    };
  }

  static void Free(Index* index) {
    if (index != nullptr) {
      sfree(index->indices);
      index->indices = nullptr;
      index->length = 0;
      sfree(index);
    }
  }
};

struct IndexPage {
  Index** indexes;
  uint32_t length;

  static IndexPage* New(Database& db) {
    Index** indexes = smalloc<Index*>(vector_num_dimension);

    #ifdef ENABLE_OMP
      #pragma omp parallel for
    #endif
    for (uint32_t key = 0; key < vector_num_dimension; key++) {
      indexes[key] = Index::New(db);
    }

    #ifdef ENABLE_OMP
      #pragma omp parallel for
    #endif
    for (uint32_t key = 0; key < vector_num_dimension; key++) {
      Index* index = indexes[key];
      std::sort(index->indices, index->indices + index->length, [&key, &db](const uint32_t& a, const uint32_t& b) {
        const Record& A = db.records[a];
        const Record& B = db.records[b];
        for (uint32_t j = key; j < vector_num_dimension; j++) {
          const float32_t Aj = A.fields[j];
          const float32_t Bj = B.fields[j];
          if (Aj != Bj) {
            return Aj < Bj;
          }
        }
        for (uint32_t j = 0; j < key; j++) {
          const float32_t Aj = A.fields[j];
          const float32_t Bj = B.fields[j];
          if (Aj != Bj) {
            return Aj < Bj;
          }
        }
        return a < b;
      });
    }

    return new IndexPage {
      .indexes = indexes,
      .length = vector_num_dimension
    };
  }

  static void Free(IndexPage* page) {
    if (page != nullptr) {
      for (uint32_t i = 0; i < page->length; i++) {
        Index::Free(page->indexes[i]);
      }
      sfree(page->indexes);
      sfree(page);
    }
  }
};

struct Links {
  typedef std::pair<uint32_t, score_t> link_t;
  link_t links[k_nearest_neighbors];
  uint16_t length;

  void clear() {
    length = 0;
  }

  bool has(uint32_t index) const {
    for (uint32_t i = 0; i < length; i++) {
      if (links[i].first == index) {
        return true;
      }
    }
    return false;
  }

  /* matches link_t.index == index */
  const link_t& get(uint32_t index) const {
    for (uint32_t i = 0; i < length; i++) {
      if (links[i].first == index) {
        return links[i];
      }
    }
    throw std::out_of_range("index " + std::to_string(index) + " is not in Links");
  }

  /* returns links[pos] */
  const link_t& at(uint32_t pos) const {
    if (pos >= length) {
      throw std::out_of_range("pos " + std::to_string(pos) + " is too large for Links");
    }

    return links[pos];
  }

  void write(uint32_t pos, uint32_t index, score_t score) {
    links[pos] = {index, score};
  }

  void sort() {
    std::sort(links, links + length, [](const link_t& a, const link_t& b) {
      if (a.second != b.second) {
        return a.second < b.second;
      } else {
        return a.first < b.first;
      }
    });
  }

  void push(uint32_t index, score_t score) {
    if (full()) {
      uint32_t last = length - 1;
      if (score < links[last].second) {
        write(last, index, score);
        sort();
      }
    } else {
      write(length++, index, score);
      sort();
    }
  }

  uint32_t size() const {
    return length;
  }

  bool full() const {
    return length == k_nearest_neighbors;
  }

  void dump() {
    for (uint32_t i = 0; i < length; i++) {
      std::cout << " (" << links[i].first << " ; " << links[i].second << ")";
    }
  }
};

struct Graph {
  Links* adiacency;
  uint32_t length;

  static Graph* New(Database& db) {
    Links* adiacency = smalloc<Links>(db.length);
    
    for (uint32_t i = 0; i < db.length; i++) {
      adiacency[i].clear();
    }
    
    return new Graph {
      .adiacency = adiacency,
      .length = db.length
    };
  }

  static void Free(Graph* graph) {
    if (graph != nullptr) {
      delete graph->adiacency;
      delete graph;
    }
  }

  void fill(const Database& db, const IndexPage& page) {
    // iterate over rows
    for (uint32_t i = 0; i < page.length; i++) {
      const Index& row = *page.indexes[i];
      uint32_t n_of_partitions = row.length / PARTITION_LENGTH;
      if (row.length % PARTITION_LENGTH != 0)
        n_of_partitions++;

      #ifdef ENABLE_OMP
        #pragma omp parallel for
      #endif
      for (uint32_t j = 0; j < n_of_partitions; j++) {
        uint32_t partition_start = j * PARTITION_LENGTH;
        uint32_t partition_end = partition_start + PARTITION_LENGTH;
        for (uint32_t node_a = partition_start; node_a < partition_end; node_a++) {
          for (uint32_t node_b = node_a + 1; node_b < partition_end; node_b++) {
            uint32_t index_a = row.indices[node_a];
            uint32_t index_b = row.indices[node_b];

            if (adiacency[index_a].has(index_b)) {
              if (adiacency[index_b].has(index_a)) {
              } else {
                score_t score = adiacency[index_a].get(index_b).second;
                adiacency[index_b].push(index_a, score);
              }
            } else {
              if (adiacency[index_b].has(index_a)) {
                score_t score = adiacency[index_b].get(index_a).second;
                adiacency[index_a].push(index_b, score);
              } else {
                score_t score = distance(db.records[index_a], db.records[index_b]);
                adiacency[index_a].push(index_b, score);
                adiacency[index_b].push(index_a, score);
              }
            }
          }
        }
      }
    }
  }

  void dump() {
    for (uint32_t i = 0; i < length; i++) {
      std::cout << i << " :=";
      adiacency[i].dump();
      std::cout << std::endl;
    }
  }
};

int main(int argc, char** args) {
  omp_set_num_threads(omp_get_max_threads());

  std::string db_path = "dummy-data.bin";
  std::string qs_path = "dummy-queries.bin";

  if (argc > 1) {
    db_path = std::string(args[1]);

    if (argc > 2) {
      qs_path = std::string(args[2]);
    }
  }

  Database db = ReadDatabase(db_path);
  LogTime("Read DB");

  QuerySet qs = ReadQuerySet(qs_path);
  LogTime("Read QS");

  IndexPage* page = IndexPage::New(db);
  LogTime("Built Index Page");

  Graph* graph = Graph::New(db);
  LogTime("Initialized Graph");

  graph->fill(db, *page);
  LogTime("Built Graph");
  
  // graph->dump();
  // LogTime("Dumped Graph");

  IndexPage::Free(page);
  LogTime("Freed Index Page");

  Graph::Free(graph);
  LogTime("Freed Graph");

  FreeDatabase(db);
  LogTime("Freed DB");

  FreeQuerySet(qs);
  LogTime("Freed QS");
}

#include <sigmod/config.hh>
#include <sigmod/query_set.hh>
#include <sigmod/database.hh>
#include <sigmod/memory.hh>
#include <sigmod/scoreboard.hh>
#include <sigmod/random.hh>
#include <sigmod/flags.hh>
#include <omp.h>
#include <algorithm>
#include <cassert>

struct Hyperplane {
  float32_t fields[vector_num_dimension];

  static Hyperplane* From(const Record& a, const Record& b) {
    Hyperplane* hyperplane = smalloc<Hyperplane>();
    for (uint32_t i = 0; i < vector_num_dimension; i++) {
      hyperplane->fields[i] = a.fields[i] - b.fields[i];
    }
    return hyperplane;
  }

  /* True := Left; False := Right */
  bool sideof(const Record& vector) {
    float32_t sum = 0.0;
    for (uint32_t i = 0; i < vector_num_dimension; i++) {
      sum += fields[i] * vector.fields[i];
    }
    return sum >= 0;
  }

  /* True := Left; False := Right */
  bool sideof(const Query& vector) {
    float32_t sum = 0.0;
    for (uint32_t i = 0; i < vector_num_dimension; i++) {
      sum += fields[i] * vector.fields[i];
    }
    return sum >= 0;
  }

  static void Free(Hyperplane*& hyperplane) {
    if (hyperplane != nullptr) {
      sfree(hyperplane);
      hyperplane = nullptr;
    }
  }
};

struct Index {
  uint32_t* indices;
  uint32_t length;

  static Index* New(uint32_t length, uint32_t* src = nullptr) {
    uint32_t* indices = smalloc<uint32_t>(length);
    if (src == nullptr) {
      for (uint32_t i = 0; i < length; i++) {
        indices[i] = i;
      }
    } else {
      std::memcpy(indices, src, sizeof(uint32_t) * length);
    }
    return new Index {
      indices, length
    };
  }

  static void Free(Index*& index) {
    if (index != nullptr) {
      sfree(index->indices);
      index->indices = nullptr;
      index->length = 0;
      sfree(index);
      index = nullptr;
    }
  }

  uint32_t* begin() {
    return indices;
  }

  uint32_t* end() {
    return indices + length;
  }
};

struct Node {
  enum {LEAF, INTERNAL} type;

  union {
    struct {
      Index* by_C;
      Index* by_T;
    } leaf;
    struct {
      Hyperplane* hyperplane;
      Node* left;
      Node* right;
    } internal;
  };

  static Node* New(const Database& db, Index& index, uint32_t start, uint32_t end) {
    uint32_t length = end - start;
    if (length <= 100) {
      Index* by_C = Index::New(length, index.begin());
      std::sort(by_C->begin(), by_C->end(), [&db](const uint32_t&a, const uint32_t& b) {
        const Record& A = db.records[a];
        const Record& B = db.records[b];
        if (A.C != B.C) {
          return A.C < B.C;
        } else {
          return a < b;
        }
      });

      Index* by_T = Index::New(length, index.begin());
      std::sort(by_T->begin(), by_T->end(), [&db](const uint32_t&a, const uint32_t& b) {
        const Record& A = db.records[a];
        const Record& B = db.records[b];
        if (A.T != B.T) {
          return A.T < B.T;
        } else {
          return a < b;
        }
      });
      return new Node {
        .type = LEAF,
        .leaf = {by_C, by_T}
      };
    } else {
      uint32_t x = index.indices[RandomUINT32T(start, end)];
      uint32_t y = index.indices[RandomUINT32T(start, end)];
      while(x == y)
        y = index.indices[RandomUINT32T(start, end)];

      Hyperplane* hyperplane = Hyperplane::From(db.records[x], db.records[y]);

      uint32_t i = start;
      uint32_t j = end - 1;
      bool side_of_i = hyperplane->sideof(db.records[index.indices[i]]);
      bool side_of_j = hyperplane->sideof(db.records[index.indices[j]]);

      while(i < j) {
        while(side_of_i == true) {
          side_of_i = hyperplane->sideof(db.records[index.indices[++i]]);
        }
        while(side_of_j == false) {
          side_of_j = hyperplane->sideof(db.records[index.indices[--j]]);
        }
        if (side_of_i == false && side_of_j == true) {
          std::swap(index.indices[i], index.indices[j]);
          i++; j--;
          side_of_i = hyperplane->sideof(db.records[index.indices[i]]);
          if (i != j) {
            side_of_j = hyperplane->sideof(db.records[index.indices[j]]);
          } else {
            side_of_j = side_of_i;
          }
        }
      }

      uint32_t middle = i;
      bool side_of_middle = side_of_i;
      while(side_of_middle != false) {
        side_of_middle = hyperplane->sideof(db.records[index.indices[++middle]]);
      }
      
      assert(hyperplane->sideof(db.records[index.indices[middle]]) == false);

      Node* left = Node::New(db, index, start, middle);
      Node* right = Node::New(db, index, middle, end);

      return new Node {
        .type = INTERNAL,
        .internal = {
          .hyperplane = hyperplane,
          .left = left,
          .right = right
        }
      };
    }
  }
  
  static void Free(Node*& node) {
    if (node != nullptr) {
      switch (node->type) {
        case LEAF: {
          Index::Free(node->leaf.by_C);
          Index::Free(node->leaf.by_T);
        }; break;
        case INTERNAL: {
          Node::Free(node->internal.left);
          Node::Free(node->internal.right);
          Hyperplane::Free(node->internal.hyperplane);
        }; break;
      }
      sfree(node);
      node = nullptr;
    }
  }

  void search(const Database& db, const Query& query, Scoreboard& scoreboard) {
    switch (type) {
      case LEAF: {
        for (uint32_t i = 0; leaf.by_C->length; i++) {
          const uint32_t index = leaf.by_C->indices[i];
          const score_t score = distance(db.records[index], query);
          scoreboard.push(index, score);
        }
      }; break;
      case INTERNAL: {
        bool side_of_query = internal.hyperplane->sideof(query);
        if (side_of_query == true) {
          internal.left->search(db, query, scoreboard);
          if (scoreboard.not_full()) {
            internal.right->search(db, query, scoreboard);
          }
        } else {
          internal.right->search(db, query, scoreboard);
          if (scoreboard.not_full()) {
            internal.left->search(db, query, scoreboard);
          }
        }
      }; break;
    }
  }
};

struct Tree {
  Node* root;

  static Tree* New(const Database& db) {
    Index* index = Index::New(db.length);
    Node* root = Node::New(db, *index, 0, db.length);
    Index::Free(index);
    return new Tree {
      root
    };
  }

  static void Free(Tree*& tree) {
    if (tree != nullptr) {
      Node::Free(tree->root);
      sfree(tree);
      tree = nullptr;
    }
  }

  void search(const Database& db, const Query& query, Scoreboard& scoreboard) {
    root->search(db, query, scoreboard);
  }
};

void Flush(Scoreboard& scoreboard) {
  uint32_t rank = scoreboard.size();
  while(scoreboard.size() > 0) {
    std::cout << "#" << rank << " := " << scoreboard.top().index << ", d := "<< scoreboard.top().score << std::endl;
    scoreboard.pop();
    rank--;
  }
}

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
  
  Tree* tree = Tree::New(db);
  LogTime("Built Tree");

  Scoreboard scoreboard;
  LogTime("Init Scoreboard");

  tree->search(db, qs.queries[17], scoreboard);
  LogTime("Filled Scoreboard");
  
  Flush(scoreboard);
  LogTime("Flushed Scoreboard");
  
  Tree::Free(tree);
  LogTime("Freed Tree");

  FreeDatabase(db);
  LogTime("Freed DB");

  FreeQuerySet(qs);
  LogTime("Freed QS");
}

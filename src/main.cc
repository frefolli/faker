#include <sigmod/query_set.hh>
#include <sigmod/config.hh>
#include <sigmod/database.hh>
#include <sigmod/metrics.hh>
#include <sigmod/memory.hh>
#include <sigmod/flags.hh>
#include <sigmod/scoreboard.hh>
#include <sigmod.hh>
#include <omp.h>
#include <algorithm>
#include <cassert>

struct Tree {
  score_t* metrics;
  uint32_t* indexes;
  uint32_t length;
  uint32_t start;
  uint32_t end;
};

inline bool IsNotLeaf(uint32_t node_start, uint32_t node_end) {
  return (node_end - node_start > 2 * k_nearest_neighbors);
}

inline uint32_t GetMiddle(uint32_t node_start, uint32_t node_end) {
  return (node_start + node_end) / 2;
}

void BuildNode(Tree& tree, Database& db, uint32_t node_start, uint32_t node_end) {
  if (IsNotLeaf(node_start, node_end)) {
    uint32_t root = GetMiddle(node_start, node_end);
    BuildNode(tree, db, node_start, root);
    BuildNode(tree, db, root + 1, node_end);
  } else {
    std::sort(tree.indexes + node_start, tree.indexes + node_end,
              [&tree, &db](const uint32_t&a, const uint32_t&b) {
      return db.records[tree.start + a].T < db.records[tree.start + b].T;
    });
  }
}

void BuildTree(Tree& tree, Database& db, uint32_t tree_start, uint32_t tree_end) {
  uint32_t length = tree_end - tree_start;
  tree = {
    .metrics = smalloc<score_t>(length),
    .indexes = smalloc<uint32_t>(length),
    .length = length,
    .start = tree_start,
    .end = tree_end
  };

  #pragma omp parallel for
  for (uint32_t i = 0; i < length; i++) {
    #ifdef USE_FIRST_METRIC
      tree.metrics[i] = first_metric(db.records[tree.start + i]);
    #endif
    #ifdef USE_SECOND_METRIC
      tree.metrics[i] = second_metric(db.records[tree.start + i]);
    #endif
    tree.indexes[i] = i;
  }

  std::sort(tree.indexes, tree.indexes + length,
            [&tree](const uint32_t&a, const uint32_t&b) {
    return tree.metrics[a] < tree.metrics[b];
  });

  BuildNode(tree, db, 0, length);
}

inline void TryCandidate(Tree& tree, Database& db,
                         Scoreboard& board, Query& query,
                         uint32_t i) {
  uint32_t index = tree.start + tree.indexes[i];
  score_t score = distance(db.records[index], query);
  board.push(index, score);
}

uint32_t TRESHOLD = 10 * k_nearest_neighbors;
void SearchInNode(Tree& tree, Database& db,
                  Scoreboard& board, uint32_t& evaluated,
                  Query& query, score_t query_metric,
                  uint32_t node_start, uint32_t node_end) {
  if (IsNotLeaf(node_start, node_end)) {
    uint32_t root = GetMiddle(node_start, node_end);
    TryCandidate(tree, db, board, query, root);
    evaluated++;

    if (query_metric < tree.metrics[tree.indexes[root]]) {
      SearchInNode(tree, db, board, evaluated, query, query_metric, node_start, root);
      if (board.not_full() || evaluated < TRESHOLD) {
        SearchInNode(tree, db, board, evaluated, query, query_metric, root + 1, node_end);
      }
    } else {
      SearchInNode(tree, db, board, evaluated, query, query_metric, root + 1, node_end);
      if (board.not_full() || evaluated < TRESHOLD) {
        SearchInNode(tree, db, board, evaluated, query, query_metric, node_start, root);
      }
    }
  } else {
    if (node_start < node_end)
      evaluated += node_end - node_start;
    for (uint32_t i = node_start; i < node_end; i++) {
      TryCandidate(tree, db, board, query, i);
    }
  }
}

void SearchInTree(Tree& tree, Database& db, Scoreboard& board, Query& query) {
  score_t query_metric;
  #ifdef USE_FIRST_METRIC
    query_metric = first_metric(query);
  #endif
  #ifdef USE_SECOND_METRIC
    query_metric = second_metric(query);
  #endif

  uint32_t evaluated = 0;
  SearchInNode(tree, db, board, evaluated, query, query_metric, 0, tree.length);
}

inline void TryCandidate(Database& db, Scoreboard& board,
                         Query& query, uint32_t index) {
  score_t score = distance(db.records[index], query);
  board.push(index, score);
}

void SearchExaustive(Database& db, Scoreboard& board, Query& query) {
  for (uint32_t i = 0; i < db.length; i++)
    TryCandidate(db, board, query, i);
}

struct Solution {
  uint32_t length;
  uint32_t* records;
};

void Flush(Solution& solution, Scoreboard& board) {
  assert (solution.records == nullptr);
  assert (board.size() > 0);
  solution = {
    .length = board.size(),
    .records = smalloc<uint32_t>(board.size())
  };
  uint32_t rank = board.size() - 1;
  while (board.size() > 0) {
    solution.records[rank] = board.top().index;
    board.pop();
    rank--;
  }
}

void Free(Solution& solution) {
  free(solution.records);
  solution.records = nullptr;
  solution.length = 0;
}

double Recall(const Solution& expected, const Solution& obtained) {
  double recall = 0;
  for (uint32_t i = 0; i < obtained.length; i++) {
    for (uint32_t j = 0; j < expected.length; j++) {
      if (obtained.records[i] == expected.records[j]) {
        recall += 1;
      }
    }
  }
  return recall / (double) k_nearest_neighbors;
}

int main(int argc, char** args) {
  if (argc > 1) {
    TRESHOLD = std::stoi(args[1]) * k_nearest_neighbors;
  }

  uint32_t MAX_QUERIES = 1;
  if (argc > 2) {
    MAX_QUERIES = std::stoi(args[2]);
  }

  Database db = ReadDatabase("dummy-data.bin");
  QuerySet qs = ReadQuerySet("dummy-queries.bin");

  Tree tree;
  BuildTree(tree, db, 0, db.length);

  double recall = 0;
  uint32_t n_of_queries = std::min(qs.length, MAX_QUERIES);
  for (uint32_t j = 0; j < n_of_queries; j++) {
    Query& query = qs.queries[j];
    Scoreboard board_t;
    SearchInTree(tree, db, board_t, query);

    Scoreboard board_e;
    SearchExaustive(db, board_e, query);

    Solution expected, obtained;
    Flush(expected, board_e);
    Flush(obtained, board_t);

    recall += Recall(expected, obtained);

    Free(expected);
    Free(obtained);
    board_t.clear();
    board_e.clear();
  }
  recall /= n_of_queries;

  #ifdef USE_FIRST_METRIC
    std::cout << "First,";
  #endif
  #ifdef USE_SECOND_METRIC
    std::cout << "Second,";
  #endif
  std::cout << db.length << "," << qs.length << "," << n_of_queries << "," << recall << "," << TRESHOLD << std::endl;

  FreeDatabase(db);
  FreeQuerySet(qs);
}

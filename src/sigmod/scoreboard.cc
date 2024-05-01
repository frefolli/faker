#include <sigmod/scoreboard.hh>
#include <sigmod/flags.hh>

uint32_t Scoreboard::size() const {
    return board.size();
}

const Candidate& Scoreboard::furthest() const {
    return board.back();
}

const Candidate& Scoreboard::nearest() const {
    return board[0];
}

void Scoreboard::pop() {
    board.pop_back();
}

void Scoreboard::add(const uint32_t index, const score_t score) {
    #ifdef SCOREBOARD_ALWAYS_CHECK_DUPLICATES
    if (has(index))
      return;
    #endif

    auto it = board.begin();
    while(it != board.end() && it->score < score)
        it++;

    board.emplace(it, index, score);
}

bool Scoreboard::has(const uint32_t index) const {
    for (auto it = board.begin(); it != board.end(); it++) {
        if (it->index == index)
            return true;
    }
    return false;
}

bool Scoreboard::empty() const {
    return board.size() == 0;
}

Candidate::Candidate(const uint32_t index, const score_t score) :
    index(index), score(score) {}

void Scoreboard::consider(const Candidate& candidate) {
    if (full()) {
        if (candidate.score < board.back().score) {
            pop();
            add(candidate.index, candidate.score);
        }
    } else {
        add(candidate.index, candidate.score);
    }
}

void Scoreboard::update(const Scoreboard& input) {
    for (auto candidate : input.board) {
        consider(candidate);
    }
}

bool Scoreboard::full() const {
    return board.size() == k_nearest_neighbors;
}

void Scoreboard::clear() {
  board.resize(0);
}
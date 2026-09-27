#include "tt.h"
#include <cstring>

namespace Andromeda {

TranspositionTable TT;

void TranspositionTable::resize(size_t mb) {
    delete[] entries_;
    size_t bytes = mb * 1024 * 1024;
    num_entries_ = bytes / sizeof(TTEntry);
    if (num_entries_ == 0) num_entries_ = 1024;
    entries_ = new TTEntry[num_entries_];
    clear();
}

void TranspositionTable::clear() {
    if (entries_) {
        std::memset(entries_, 0, num_entries_ * sizeof(TTEntry));
    }
    age_ = 0;
}

bool TranspositionTable::probe(Key key, TTEntry& entry) const {
    if (!entries_ || num_entries_ == 0) return false;
    size_t idx = key % num_entries_;
    const TTEntry& e = entries_[idx];
    if (e.key == key) {
        entry = e;
        return true;
    }
    return false;
}

void TranspositionTable::store(Key key, Move move, Value score, int16_t eval, Depth depth, Bound bound) {
    if (!entries_ || num_entries_ == 0) return;
    size_t idx = key % num_entries_;
    TTEntry& e = entries_[idx];

    if (e.key != key || depth >= e.depth || bound == BOUND_EXACT) {
        e.key = key;
        if (!move.is_none()) e.move = move;
        e.score = score;
        e.eval = eval;
        e.depth = depth;
        e.bound = bound;
        e.age = age_;
    }
}

int TranspositionTable::hashfull() const {
    if (!entries_ || num_entries_ == 0) return 0;
    int sampled = 1000;
    int used = 0;
    for (int i = 0; i < sampled && i < static_cast<int>(num_entries_); ++i) {
        if (entries_[i].key != 0 && entries_[i].age == age_) {
            used++;
        }
    }
    return (used * 1000) / sampled;
}

} // namespace Andromeda

#pragma once
#include "type.h"
#include <vector>

namespace Andromeda {

enum Bound : uint8_t {
    BOUND_NONE = 0,
    BOUND_UPPER = 1,
    BOUND_LOWER = 2,
    BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

struct TTEntry {
    Key key;
    Move move;
    Value score;
    int16_t eval;
    Depth depth;
    Bound bound;
    uint8_t age;
};

class TranspositionTable {
public:
    TranspositionTable() : entries_(nullptr), num_entries_(0), age_(0) {}
    ~TranspositionTable() { delete[] entries_; }

    void resize(size_t mb);
    void clear();
    void new_search() { age_++; }

    bool probe(Key key, TTEntry& entry) const;
    void store(Key key, Move move, Value score, int16_t eval, Depth depth, Bound bound);

    int hashfull() const;

private:
    TTEntry* entries_;
    size_t num_entries_;
    uint8_t age_;
};

extern TranspositionTable TT;

} // namespace Andromeda

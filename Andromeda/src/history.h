#pragma once
#include "type.h"
#include <cstring>

namespace Andromeda {

class HistoryTable {
public:
    HistoryTable() { clear(); }

    void clear() {
        std::memset(history_, 0, sizeof(history_));
        std::memset(killers_, 0, sizeof(killers_));
    }

    int get(Color c, Square from, Square to) const {
        return history_[c][from][to];
    }

    void update(Color c, Move m, int bonus) {
        int clamped = std::max(-2000, std::min(2000, bonus));
        history_[c][m.from()][m.to()] += clamped - (history_[c][m.from()][m.to()] * std::abs(clamped) / 2000);
    }

    void add_killer(int ply, Move m) {
        if (ply < MAX_PLY) {
            if (killers_[ply][0] != m) {
                killers_[ply][1] = killers_[ply][0];
                killers_[ply][0] = m;
            }
        }
    }

    Move get_killer(int ply, int idx) const {
        if (ply < MAX_PLY && idx < 2) return killers_[ply][idx];
        return Move::none();
    }

private:
    int history_[COLOR_NB][SQUARE_NB][SQUARE_NB];
    Move killers_[MAX_PLY][2];
};

} // namespace Andromeda

#pragma once
#include "position.h"
#include "tt.h"
#include "timeman.h"
#include "history.h"
#include <atomic>
#include <cstdint>
#include <vector>

namespace Andromeda {

struct SearchLimits {
    int wtime = 0;
    int btime = 0;
    int winc = 0;
    int binc = 0;
    int movestogo = 0;
    int depth = 0;
    int nodes = 0;
    int mate = 0;
    int movetime = 0;
    bool infinite = false;
    std::vector<Move> searchmoves;
};

struct SearchStack {
    Move current_move;
    Move excluded_move;
    Value static_eval = VALUE_NONE;
    int stat_score = 0;
    int ply = 0;
    bool in_check = false;
};

class Searcher {
public:
    Searcher();

    void start_search(Position& pos, const SearchLimits& limits);
    void stop();

    bool is_searching() const { return searching_; }
    uint64_t nodes() const { return nodes_; }

private:
    Value search(Position& pos, SearchStack* ss, Value alpha, Value beta, Depth depth, bool cut_node);
    Value qsearch(Position& pos, SearchStack* ss, Value alpha, Value beta);

    void check_time();
    bool should_stop() const;

    std::atomic<bool> stop_flag_;
    std::atomic<bool> searching_;
    uint64_t nodes_;
    int seldepth_;
    TimeManager time_man_;
    HistoryTable history_;
    SearchLimits limits_;
    Move best_move_;
    Move completed_best_move_;
    Value completed_score_;
    int root_depth_;
};

extern Searcher GlobalSearcher;

} // namespace Andromeda

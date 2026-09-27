#pragma once
#include "position.h"
#include "history.h"
#include "movegen.h"

namespace Andromeda {

enum Stage {
    STAGE_TT_MOVE,
    STAGE_GEN_CAPTURES,
    STAGE_GOOD_CAPTURES,
    STAGE_KILLER_1,
    STAGE_KILLER_2,
    STAGE_GEN_QUIETS,
    STAGE_QUIETS,
    STAGE_BAD_CAPTURES,
    STAGE_DONE
};

class MovePicker {
public:
    MovePicker(Position& pos, Move tt_move, const HistoryTable& history, int ply);

    Move next_move();

private:
    void score_captures();
    void score_quiets();

    Position& pos_;
    Move tt_move_;
    const HistoryTable& history_;
    int ply_;
    Stage stage_;

    Move moves_[256];
    int scores_[256];
    int size_;
    int cur_idx_;
};

} // namespace Andromeda

#include "movepick.h"
#include <algorithm>

namespace Andromeda {

namespace {
    bool legal_move_exists(Position& pos, Move candidate) {
        if (candidate == Move::none())
            return false;
        Move legal[256];
        const int count = generate_legal_moves(pos, legal);
        for (int i = 0; i < count; ++i)
            if (legal[i] == candidate)
                return true;
        return false;
    }

    const int VictimScores[PIECE_TYPE_NB] = { 0, 100, 300, 310, 500, 900, 10000 };
}

MovePicker::MovePicker(Position& pos, Move tt_move, const HistoryTable& history, int ply)
    : pos_(pos), tt_move_(tt_move), history_(history), ply_(ply), stage_(STAGE_TT_MOVE), size_(0), cur_idx_(0) {
    if (tt_move_ == Move::none()) {
        stage_ = STAGE_GEN_CAPTURES;
    }
}

void MovePicker::score_captures() {
    for (int i = 0; i < size_; ++i) {
        Move m = moves_[i];
        Piece victim = pos_.piece_on(m.to());
        Piece attacker = pos_.piece_on(m.from());
        int score = 0;
        if (victim != NO_PIECE) {
            score = VictimScores[type_of(victim)] * 10 - VictimScores[type_of(attacker)];
        } else if (m.type() == EN_PASSANT) {
            score = VictimScores[PAWN] * 10 - VictimScores[PAWN];
        }
        if (m.type() == PROMOTION) {
            score += VictimScores[m.promotion_type()] * 5;
        }
        scores_[i] = score;
    }
}

void MovePicker::score_quiets() {
    Color us = pos_.side_to_move();
    for (int i = 0; i < size_; ++i) {
        Move m = moves_[i];
        scores_[i] = history_.get(us, m.from(), m.to());
    }
}

Move MovePicker::next_move() {
    while (stage_ != STAGE_DONE) {
        switch (stage_) {
            case STAGE_TT_MOVE:
                stage_ = STAGE_GEN_CAPTURES;
                if (legal_move_exists(pos_, tt_move_))
                    return tt_move_;
                break;

            case STAGE_GEN_CAPTURES: {
                Move* end = generate_moves<GEN_CAPTURES>(pos_, moves_);
                size_ = static_cast<int>(end - moves_);
                cur_idx_ = 0;
                score_captures();
                stage_ = STAGE_GOOD_CAPTURES;
                break;
            }

            case STAGE_GOOD_CAPTURES:
                while (cur_idx_ < size_) {
                    int best_idx = cur_idx_;
                    for (int i = cur_idx_ + 1; i < size_; ++i) {
                        if (scores_[i] > scores_[best_idx]) best_idx = i;
                    }
                    std::swap(moves_[cur_idx_], moves_[best_idx]);
                    std::swap(scores_[cur_idx_], scores_[best_idx]);
                    Move m = moves_[cur_idx_++];
                    if (m != tt_move_) return m;
                }
                stage_ = STAGE_KILLER_1;
                break;

            case STAGE_KILLER_1: {
                stage_ = STAGE_KILLER_2;
                Move k1 = history_.get_killer(ply_, 0);
                if (k1 != Move::none() && k1 != tt_move_ && !pos_.is_capture(k1)
                    && legal_move_exists(pos_, k1)) {
                    return k1;
                }
                break;
            }

            case STAGE_KILLER_2: {
                stage_ = STAGE_GEN_QUIETS;
                Move k2 = history_.get_killer(ply_, 1);
                if (k2 != Move::none() && k2 != tt_move_ && !pos_.is_capture(k2)
                    && legal_move_exists(pos_, k2)) {
                    return k2;
                }
                break;
            }

            case STAGE_GEN_QUIETS: {
                Move* end = generate_moves<GEN_QUIETS>(pos_, moves_);
                size_ = static_cast<int>(end - moves_);
                cur_idx_ = 0;
                score_quiets();
                stage_ = STAGE_QUIETS;
                break;
            }

            case STAGE_QUIETS:
                while (cur_idx_ < size_) {
                    int best_idx = cur_idx_;
                    for (int i = cur_idx_ + 1; i < size_; ++i) {
                        if (scores_[i] > scores_[best_idx]) best_idx = i;
                    }
                    std::swap(moves_[cur_idx_], moves_[best_idx]);
                    std::swap(scores_[cur_idx_], scores_[best_idx]);
                    Move m = moves_[cur_idx_++];
                    if (m != tt_move_ && m != history_.get_killer(ply_, 0) && m != history_.get_killer(ply_, 1)) {
                        return m;
                    }
                }
                stage_ = STAGE_DONE;
                break;

            case STAGE_BAD_CAPTURES:
            case STAGE_DONE:
                return Move::none();
        }
    }
    return Move::none();
}

} // namespace Andromeda

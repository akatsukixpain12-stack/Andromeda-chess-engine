#include "search.h"
#include "evaluate.h"
#include "movepick.h"
#include <iostream>
#include <iomanip>
#include <cmath>

namespace Andromeda {

Searcher GlobalSearcher;

Searcher::Searcher() : stop_flag_(false), searching_(false), nodes_(0), seldepth_(0) {}

void Searcher::stop() {
    stop_flag_ = true;
}

void Searcher::check_time() {
    if (time_man_.should_stop(static_cast<int>(nodes_), 0)) {
        stop_flag_ = true;
    }
}

Value Searcher::qsearch(Position& pos, SearchStack* ss, Value alpha, Value beta) {
    if (stop_flag_) return VALUE_ZERO;
    nodes_++;

    if (pos.is_draw(0)) return VALUE_DRAW;

    Value stand_pat = evaluate(pos);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    Move moves[256];
    Move* end = generate_moves<GEN_CAPTURES>(pos, moves);

    for (Move* m = moves; m < end; ++m) {
        // Delta pruning
        if (stand_pat + 900 < alpha && !pos.in_check()) {
            continue;
        }

        StateInfo si;
        if (!pos.do_move(*m, si)) continue;

        Value score = -qsearch(pos, ss + 1, -beta, -alpha);
        pos.undo_move(*m);

        if (stop_flag_) return VALUE_ZERO;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

Value Searcher::search(Position& pos, SearchStack* ss, Value alpha, Value beta, Depth depth, bool cut_node) {
    if (stop_flag_) return VALUE_ZERO;

    if (nodes_ % 2048 == 0) {
        check_time();
    }

    int ply = static_cast<int>(ss - (SearchStack*)nullptr); // relative offset or stack depth
    if (depth <= 0) {
        return qsearch(pos, ss, alpha, beta);
    }

    nodes_++;
    bool is_root = (depth >= limits_.depth && ply <= 1);
    bool in_check = pos.in_check();

    if (!is_root && pos.is_draw(ply)) {
        return VALUE_DRAW;
    }

    // TT Probe
    TTEntry tte;
    Move tt_move = Move::none();
    Key pos_key = pos.key();
    if (TT.probe(pos_key, tte)) {
        tt_move = tte.move;
        if (!is_root && tte.depth >= depth) {
            if (tte.bound == BOUND_EXACT) return tte.score;
            if (tte.bound == BOUND_LOWER && tte.score >= beta) return beta;
            if (tte.bound == BOUND_UPPER && tte.score <= alpha) return alpha;
        }
    }

    // Static evaluation
    Value static_eval = evaluate(pos);
    ss->static_eval = static_eval;

    // Null Move Pruning
    if (!is_root && !in_check && depth >= 3 && static_eval >= beta) {
        StateInfo nsi;
        pos.do_null_move(nsi);
        Value null_score = -search(pos, ss + 1, -beta, -beta + 1, depth - 1 - 2, !cut_node);
        pos.undo_null_move();
        if (null_score >= beta) {
            return (null_score >= VALUE_MATE_IN_MAX_PLY) ? beta : null_score;
        }
    }

    // Move loop
    MovePicker picker(pos, tt_move, history_, ply);
    Move move;
    Move best_move = Move::none();
    Value best_score = -VALUE_INFINITE;
    int moves_played = 0;
    Bound bound = BOUND_UPPER;

    while ((move = picker.next_move()) != Move::none()) {
        StateInfo si;
        if (!pos.do_move(move, si)) continue;

        moves_played++;
        ss->current_move = move;

        Value score = VALUE_ZERO;
        Depth next_depth = depth - 1;

        // Late Move Reductions (LMR)
        if (moves_played > 3 && depth >= 3 && !in_check && !pos.is_capture(move) && move.type() != PROMOTION) {
            int reduction = 1 + static_cast<int>(std::log(depth) * std::log(moves_played) / 2.0);
            Depth reduced_depth = std::max(1, next_depth - reduction);
            score = -search(pos, ss + 1, -alpha - 1, -alpha, reduced_depth, true);
            if (score > alpha && reduced_depth < next_depth) {
                score = -search(pos, ss + 1, -alpha - 1, -alpha, next_depth, !cut_node);
            }
        } else if (moves_played > 1) {
            // PVS search
            score = -search(pos, ss + 1, -alpha - 1, -alpha, next_depth, !cut_node);
            if (score > alpha && score < beta) {
                score = -search(pos, ss + 1, -beta, -alpha, next_depth, false);
            }
        } else {
            score = -search(pos, ss + 1, -beta, -alpha, next_depth, false);
        }

        pos.undo_move(move);

        if (stop_flag_) return VALUE_ZERO;

        if (score > best_score) {
            best_score = score;
            best_move = move;

            if (score > alpha) {
                alpha = score;
                bound = BOUND_EXACT;

                if (score >= beta) {
                    bound = BOUND_LOWER;
                    if (!pos.is_capture(move)) {
                        history_.add_killer(ply, move);
                        history_.update(pos.side_to_move(), move, depth * depth);
                    }
                    break;
                }
            }
        }
    }

    if (moves_played == 0) {
        if (in_check) return -VALUE_MATE + ply;
        return VALUE_DRAW;
    }

    TT.store(pos_key, best_move, best_score, static_eval, depth, bound);
    return best_score;
}

void Searcher::start_search(Position& pos, const SearchLimits& limits) {
    searching_ = true;
    stop_flag_ = false;
    limits_ = limits;
    nodes_ = 0;
    seldepth_ = 0;

    time_man_.init(pos.side_to_move(), limits.wtime, limits.btime, limits.winc, limits.binc, limits.movestogo, limits.movetime);
    time_man_.start();

    TT.new_search();
    SearchStack stack[MAX_PLY];
    std::memset(stack, 0, sizeof(stack));

    int max_depth = limits.depth > 0 ? limits.depth : MAX_DEPTH;
    Move best_move = Move::none();
    Value best_score = VALUE_ZERO;

    for (int d = 1; d <= max_depth; ++d) {
        Value score = search(pos, stack + 1, -VALUE_INFINITE, VALUE_INFINITE, d, false);

        if (stop_flag_ && d > 1) break;

        TTEntry tte;
        if (TT.probe(pos.key(), tte)) {
            best_move = tte.move;
            best_score = tte.score;
        }

        int64_t elapsed = time_man_.elapsed_ms();
        uint64_t nps = elapsed > 0 ? (nodes_ * 1000 / elapsed) : nodes_ * 1000;

        std::cout << "info depth " << d
                  << " score cp " << best_score
                  << " nodes " << nodes_
                  << " nps " << nps
                  << " time " << elapsed
                  << " hashfull " << TT.hashfull()
                  << std::endl;

        if (time_man_.should_stop(static_cast<int>(nodes_), d)) {
            break;
        }
    }

    if (best_move == Move::none()) {
        Move legal[256];
        int count = generate_legal_moves(pos, legal);
        if (count > 0) best_move = legal[0];
    }

    // Print best move in UCI format
    char f1 = 'a' + file_of(best_move.from());
    char r1 = '1' + rank_of(best_move.from());
    char f2 = 'a' + file_of(best_move.to());
    char r2 = '1' + rank_of(best_move.to());
    std::string move_str = {f1, r1, f2, r2};
    if (best_move.type() == PROMOTION) {
        char promo = 'q';
        if (best_move.promotion_type() == KNIGHT) promo = 'n';
        if (best_move.promotion_type() == BISHOP) promo = 'b';
        if (best_move.promotion_type() == ROOK) promo = 'r';
        move_str += promo;
    }

    std::cout << "bestmove " << move_str << std::endl;
    searching_ = false;
}

} // namespace Andromeda

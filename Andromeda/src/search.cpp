#include "search.h"
#include "evaluate.h"
#include "movepick.h"
#include "uci.h"

#include <algorithm>
#include <string>

namespace Andromeda {

namespace {
constexpr int RAZOR_MARGIN = 180;
constexpr int FUTILITY_MARGIN = 105;
constexpr int ASPIRATION_DELTA = 24;
constexpr int MAX_QDEPTH = 12;

inline Value value_to_tt(Value v, int ply) {
    if (v > VALUE_MATE_IN_MAX_PLY)
        return v + ply;
    if (v < -VALUE_MATE_IN_MAX_PLY)
        return v - ply;
    return v;
}

inline Value value_from_tt(Value v, int ply) {
    if (v > VALUE_MATE_IN_MAX_PLY)
        return v - ply;
    if (v < -VALUE_MATE_IN_MAX_PLY)
        return v + ply;
    return v;
}

inline std::string move_to_uci(Move m) {
    if (m.is_none())
        return "0000";

    std::string s;
    s += static_cast<char>('a' + file_of(m.from()));
    s += static_cast<char>('1' + rank_of(m.from()));
    s += static_cast<char>('a' + file_of(m.to()));
    s += static_cast<char>('1' + rank_of(m.to()));

    if (m.type() == PROMOTION) {
        char promo = 'q';
        switch (m.promotion_type()) {
            case KNIGHT: promo = 'n'; break;
            case BISHOP: promo = 'b'; break;
            case ROOK:   promo = 'r'; break;
            default:     promo = 'q'; break;
        }
        s += promo;
    }
    return s;
}
} // namespace

Searcher GlobalSearcher;

Searcher::Searcher()
    : stop_flag_(false),
      searching_(false),
      nodes_(0),
      seldepth_(0),
      best_move_(Move::none()),
      completed_best_move_(Move::none()),
      completed_score_(VALUE_ZERO),
      root_depth_(0) {}

void Searcher::stop() {
    stop_flag_ = true;
}

bool Searcher::should_stop() const {
    return stop_flag_.load(std::memory_order_relaxed)
        || (!limits_.infinite && limits_.nodes > 0
            && nodes_ >= static_cast<uint64_t>(limits_.nodes));
}

void Searcher::check_time() {
    if (should_stop())
        return;

    if (time_man_.should_stop(static_cast<int>(nodes_ & 0x7fffffff), root_depth_))
        stop_flag_ = true;
}

Value Searcher::qsearch(Position& pos, SearchStack* ss, Value alpha, Value beta) {
    if ((nodes_ & 2047ULL) == 0)
        check_time();

    if (should_stop())
        return VALUE_ZERO;

    ++nodes_;
    ss->in_check = pos.in_check();
    seldepth_ = std::max(seldepth_, ss->ply);

    if (ss->in_check) {
        Move evasions[256];
        const int count = generate_legal_moves(pos, evasions);

        if (count == 0)
            return -VALUE_MATE + ss->ply;

        Value best = -VALUE_INFINITE;
        for (int i = 0; i < count; ++i) {
            StateInfo si;
            if (!pos.do_move(evasions[i], si))
                continue;

            SearchStack child = *ss;
            child.current_move = evasions[i];
            child.ply = ss->ply + 1;
            const Value score = -qsearch(pos, &child, -beta, -alpha);
            pos.undo_move(evasions[i]);

            if (should_stop())
                return VALUE_ZERO;

            best = std::max(best, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta)
                break;
        }
        return best;
    }

    const Value stand_pat = evaluate(pos);
    if (stand_pat >= beta)
        return stand_pat;
    if (stand_pat > alpha)
        alpha = stand_pat;

    if (ss->ply >= MAX_QDEPTH)
        return alpha;

    // Quiescence must see forcing checks as well as captures.
    // Searching only captures creates large tactical horizons.
    Move legal[256];
    const int count = generate_legal_moves(pos, legal);

    for (int i = 0; i < count; ++i) {
        const Move move = legal[i];
        const bool capture = pos.is_capture(move);
        const bool promotion = move.type() == PROMOTION;

        if (!capture && !promotion) {
            StateInfo probe;
            if (!pos.do_move(move, probe))
                continue;
            const bool gives_check = pos.in_check();
            pos.undo_move(move);
            if (!gives_check)
                continue;
        }

        if (capture && !promotion
            && stand_pat + 1200 < alpha)
            continue;

        StateInfo si;
        if (!pos.do_move(move, si))
            continue;

        SearchStack child = *ss;
        child.current_move = move;
        child.ply = ss->ply + 1;

        const Value score = -qsearch(pos, &child, -beta, -alpha);
        pos.undo_move(move);

        if (should_stop())
            return VALUE_ZERO;

        if (score >= beta)
            return score;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

Value Searcher::search(Position& pos,
                       SearchStack* ss,
                       Value alpha,
                       Value beta,
                       Depth depth,
                       bool cut_node) {
    (void)cut_node;

    if ((nodes_ & 2047ULL) == 0)
        check_time();

    if (should_stop())
        return VALUE_ZERO;

    const bool is_root = (ss->ply == 0);
    const bool pv_node = (beta - alpha > 1);

    const bool in_check = pos.in_check();
    ss->in_check = in_check;
    seldepth_ = std::max(seldepth_, ss->ply);

    if (in_check && !is_root)
        ++depth;

    if (depth <= 0)
        return qsearch(pos, ss, alpha, beta);

    ++nodes_;

    if (!is_root && pos.is_draw(ss->ply))
    {
        // At Aggression 0, preserve the exact draw score. For positive
        // aggression, discourage repetitions only when the side to move
        // is not objectively worse. The bias is deliberately tiny.
        if (aggression() > 0 && evaluate(pos) >= VALUE_DRAW)
            return -static_cast<Value>((aggression() + 49) / 50);

        return VALUE_DRAW;
    }

    const Key key = pos.key();

    TTEntry tte;
    Move tt_move = Move::none();
    if (TT.probe(key, tte)) {
        tt_move = tte.move;
        if (!is_root && tte.depth >= depth) {
            const Value tt_score = value_from_tt(tte.score, ss->ply);
            if (tte.bound == BOUND_EXACT)
                return tt_score;
            if (tte.bound == BOUND_LOWER && tt_score >= beta)
                return tt_score;
            if (tte.bound == BOUND_UPPER && tt_score <= alpha)
                return tt_score;
        }
    }

    const Value static_eval = evaluate(pos);
    ss->static_eval = static_eval;

    if (!is_root && !pv_node && !in_check && depth <= 3
        && static_eval + RAZOR_MARGIN * depth < alpha) {
        SearchStack child = *ss;
        child.ply = ss->ply + 1;
        const Value razor = qsearch(pos, &child, alpha - 1, alpha);
        if (razor <= alpha)
            return razor;
    }

    if (!is_root && !pv_node && !in_check && depth >= 3
        && static_eval >= beta
        && (pos.pieces(pos.side_to_move(), PAWN)
            || pos.pieces(pos.side_to_move(), KNIGHT)
            || pos.pieces(pos.side_to_move(), BISHOP)
            || pos.pieces(pos.side_to_move(), ROOK)
            || pos.pieces(pos.side_to_move(), QUEEN))) {
        StateInfo nsi;
        pos.do_null_move(nsi);

        SearchStack child = *ss;
        child.current_move = Move::null();
        child.ply = ss->ply + 1;

        const Depth reduction = 2 + depth / 6;
        const Value null_score =
            -search(pos, &child, -beta, -beta + 1,
                    std::max<Depth>(1, depth - reduction), true);

        pos.undo_null_move();

        if (should_stop())
            return VALUE_ZERO;

        if (null_score >= beta)
            return null_score;
    }

    MovePicker picker(pos, tt_move, history_, ss->ply);
    Move best_move = Move::none();
    Value best_score = -VALUE_INFINITE;
    Bound bound = BOUND_UPPER;
    int moves_played = 0;

    while (true) {
        const Move move = picker.next_move();
        if (move == Move::none())
            break;

        StateInfo si;
        if (!pos.do_move(move, si))
            continue;

        ++moves_played;
        ss->current_move = move;

        const bool quiet = !pos.is_capture(move) && move.type() != PROMOTION;
        const bool gives_check = pos.in_check();

        if (!pv_node && !in_check && depth <= 3 && quiet
            && moves_played > 1
            && static_eval + FUTILITY_MARGIN * depth <= alpha) {
            pos.undo_move(move);
            continue;
        }

        const Depth new_depth = depth - 1;
        Value score;

        if (moves_played == 1) {
            score = -search(pos, ss + 1, -beta, -alpha, new_depth, false);
        } else {
            int reduction = 0;
            if (depth >= 3 && quiet && !in_check && !gives_check) {
                reduction = 1;
                if (depth >= 7)
                    ++reduction;
                if (moves_played >= 8)
                    ++reduction;
                if (pv_node)
                    reduction = std::max(1, reduction - 1);
            }

            const Depth reduced_depth = std::max<Depth>(1, new_depth - reduction);
            score = -search(pos, ss + 1, -alpha - 1, -alpha, reduced_depth, true);

            if (score > alpha && reduced_depth < new_depth)
                score = -search(pos, ss + 1, -alpha - 1, -alpha, new_depth, false);

            if (score > alpha && score < beta && pv_node)
                score = -search(pos, ss + 1, -beta, -alpha, new_depth, false);
        }

        pos.undo_move(move);

        if (should_stop())
            return VALUE_ZERO;

        if (score > best_score) {
            best_score = score;
            best_move = move;

            if (score > alpha) {
                alpha = score;
                bound = BOUND_EXACT;

                if (is_root)
                    best_move_ = move;

                if (score >= beta) {
                    bound = BOUND_LOWER;
                    if (quiet) {
                        history_.add_killer(ss->ply, move);
                        history_.update(pos.side_to_move(), move,
                                         std::min(2000, depth * depth * 10));
                    }
                    break;
                }
            }
        }
    }

    if (moves_played == 0) {
        if (in_check)
            return -VALUE_MATE + ss->ply;
        return VALUE_DRAW;
    }

    if (is_root && best_move != Move::none())
        best_move_ = best_move;

    TT.store(key, best_move, value_to_tt(best_score, ss->ply),
             static_eval, depth, bound);

    return best_score;
}

void Searcher::start_search(Position& pos, const SearchLimits& limits) {
    searching_ = true;
    stop_flag_ = false;
    limits_ = limits;
    nodes_ = 0;
    seldepth_ = 0;
    root_depth_ = 0;
    best_move_ = Move::none();

    time_man_.init(pos.side_to_move(), limits.wtime, limits.btime,
                   limits.winc, limits.binc, limits.movestogo,
                   limits.movetime);
    time_man_.start();
    TT.new_search();

    SearchStack stack[MAX_PLY + 4]{};
    for (int i = 0; i < MAX_PLY + 4; ++i)
        stack[i].ply = i;

    const int max_depth =
        limits.depth > 0 ? std::min(limits.depth, static_cast<int>(MAX_DEPTH))
                         : static_cast<int>(MAX_DEPTH);

    Move fallback[256];
    const int fallback_count = generate_legal_moves(pos, fallback);
    completed_best_move_ = fallback_count > 0 ? fallback[0] : Move::none();
    completed_score_ = VALUE_ZERO;

    for (int d = 1; d <= max_depth; ++d) {
        if (should_stop())
            break;

        root_depth_ = d;
        best_move_ = completed_best_move_;

        Value alpha = -VALUE_INFINITE;
        Value beta = VALUE_INFINITE;

        if (d >= 3 && completed_score_ > -VALUE_MATE_IN_MAX_PLY
            && completed_score_ < VALUE_MATE_IN_MAX_PLY) {
            alpha = std::max(-VALUE_INFINITE, completed_score_ - ASPIRATION_DELTA);
            beta = std::min(VALUE_INFINITE, completed_score_ + ASPIRATION_DELTA);
        }

        Value score = completed_score_;
        int delta = ASPIRATION_DELTA;

        while (true) {
            if (should_stop())
                break;

            score = search(pos, &stack[0], alpha, beta, d, false);

            if (should_stop())
                break;

            if (score <= alpha) {
                alpha = std::max(-VALUE_INFINITE, score - delta * 2);
                beta = std::max(beta, alpha + 1);
                delta *= 2;
                continue;
            }

            if (score >= beta) {
                beta = std::min(VALUE_INFINITE, score + delta * 2);
                alpha = std::min(alpha, beta - 1);
                delta *= 2;
                continue;
            }

            break;
        }

        if (should_stop() && d > 1)
            break;

        if (best_move_ != Move::none()) {
            completed_best_move_ = best_move_;
            completed_score_ = score;
        }

        const int64_t elapsed = time_man_.elapsed_ms();
        const uint64_t nps =
            elapsed > 0 ? (nodes_ * 1000ULL) / static_cast<uint64_t>(elapsed)
                        : nodes_ * 1000ULL;

        UCI::emit("info depth " + std::to_string(d)
            + " seldepth " + std::to_string(seldepth_)
            + " score cp " + std::to_string(completed_score_)
            + " nodes " + std::to_string(nodes_)
            + " nps " + std::to_string(nps)
            + " time " + std::to_string(elapsed)
            + " hashfull " + std::to_string(TT.hashfull()));

        if (limits.depth == 0 && !limits.infinite && elapsed >= 1
            && time_man_.should_stop(static_cast<int>(nodes_ & 0x7fffffff), d))
            break;
    }

    UCI::emit("bestmove " + move_to_uci(completed_best_move_));
    searching_ = false;
}

} // namespace Andromeda

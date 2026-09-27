#include "timeman.h"
#include <algorithm>

namespace Andromeda {

TimeManager::TimeManager() : allocated_time_ms_(0), hard_limit_ms_(0), infinite_(false) {}

void TimeManager::init(Color us, int wtime, int btime, int winc, int binc, int movestogo, int movetime) {
    if (movetime > 0) {
        allocated_time_ms_ = movetime;
        hard_limit_ms_ = movetime;
        infinite_ = false;
        return;
    }

    int my_time = (us == WHITE) ? wtime : btime;
    int my_inc = (us == WHITE) ? winc : binc;

    if (my_time <= 0) {
        allocated_time_ms_ = 1000;
        hard_limit_ms_ = 2000;
        infinite_ = false;
        return;
    }

    int moves = (movestogo > 0) ? std::min(movestogo, 40) : 30;
    allocated_time_ms_ = (my_time / moves) + (my_inc * 3 / 4);
    hard_limit_ms_ = std::min(my_time * 8 / 10, static_cast<int>(allocated_time_ms_ * 3));
    infinite_ = false;
}

void TimeManager::start() {
    start_time_ = std::chrono::steady_clock::now();
}

void TimeManager::stop() {
    // stop signal
}

int64_t TimeManager::elapsed_ms() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
}

bool TimeManager::should_stop(int nodes, Depth depth) const {
    if (infinite_) return false;
    if ((nodes & 2047) != 0) return false;
    return elapsed_ms() >= allocated_time_ms_;
}

} // namespace Andromeda

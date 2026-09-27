#pragma once
#include "type.h"
#include <chrono>

namespace Andromeda {

class TimeManager {
public:
    TimeManager();

    void init(Color us, int wtime, int btime, int winc, int binc, int movestogo, int movetime);
    bool should_stop(int nodes, Depth depth) const;
    void start();
    void stop();

    int64_t elapsed_ms() const;

private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    int64_t allocated_time_ms_;
    int64_t hard_limit_ms_;
    bool infinite_;
};

} // namespace Andromeda

#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED
#include <string>
#include <vector>
#include "position.h"
#include "search.h"
#include "ucioption.h"
namespace Andromeda {
class Engine {
public:
    Engine();
    void set_position(const std::string& fen);
    void go(SearchLimits limits);
    void stop();
    void new_game();
    Position& position(){ return pos_; }
    const OptionsMap& options() const { return options_; }
    OptionsMap& options(){ return options_; }
private:
    Position pos_;
    StateInfo state_{};
    OptionsMap options_;
};
extern Engine GlobalEngine;
}
#endif

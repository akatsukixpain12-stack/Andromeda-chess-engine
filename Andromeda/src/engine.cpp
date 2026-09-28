#include "engine.h"
namespace Andromeda {
Engine GlobalEngine;
Engine::Engine(){
    options_.set("Threads","1");
    options_.set("Hash","16");
    new_game();
}
void Engine::new_game(){ pos_.set_startpos(&state_); TT.clear(); }
void Engine::set_position(const std::string& fen){ pos_.set(fen,&state_); }
void Engine::go(SearchLimits limits){ GlobalSearcher.start_search(pos_,limits); }
void Engine::stop(){ GlobalSearcher.stop(); }
}

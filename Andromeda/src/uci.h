#pragma once
#include "position.h"
#include "search.h"
#include <string>

namespace Andromeda {

class UCI {
public:
    static void loop();
    static void initialize();
    static void process_command(const std::string& line);
    static void parse_position(Position& pos, const std::string& line, StateInfo& si);
    static void parse_go(Position& pos, const std::string& line);
    static Move parse_move(const Position& pos, const std::string& move_str);

    static void emit(const std::string& line);
    static const char* output_c_str();
};

} // namespace Andromeda

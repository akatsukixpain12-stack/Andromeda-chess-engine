#include "uci.h"
#include "attacks.h"
#include "evaluate.h"
#include "tt.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

namespace Andromeda {

namespace {
Position g_position;
StateInfo g_root_state{};
bool g_initialized = false;
std::string g_output;

void append_output(const std::string& line) {
    g_output += line;
    g_output.push_back('\n');
}
}

void UCI::initialize() {
    if (g_initialized)
        return;

    init_bitboards();
    init_attacks();
    init_zobrist();
    init_evaluation();
    TT.resize(64);
    g_position.set_startpos(&g_root_state);
    g_initialized = true;
}

void UCI::emit(const std::string& line) {
    append_output(line);
}

const char* UCI::output_c_str() {
    return g_output.c_str();
}

Move UCI::parse_move(const Position& pos, const std::string& str) {
    if (str.length() < 4)
        return Move::none();
    if (str[0] < 'a' || str[0] > 'h' || str[2] < 'a' || str[2] > 'h'
        || str[1] < '1' || str[1] > '8' || str[3] < '1' || str[3] > '8')
        return Move::none();

    const Square from = make_square(static_cast<File>(str[0] - 'a'),
                                    static_cast<Rank>(str[1] - '1'));
    const Square to = make_square(static_cast<File>(str[2] - 'a'),
                                  static_cast<Rank>(str[3] - '1'));

    Move legal[256];
    Position temp_pos = pos;
    const int count = generate_legal_moves(temp_pos, legal);

    for (int i = 0; i < count; ++i) {
        const Move m = legal[i];
        if (m.from() != from || m.to() != to)
            continue;

        if (m.type() == PROMOTION) {
            if (str.length() < 5)
                continue;

            const char promo = static_cast<char>(std::tolower(str[4]));
            PieceType pt = QUEEN;
            if (promo == 'n') pt = KNIGHT;
            else if (promo == 'b') pt = BISHOP;
            else if (promo == 'r') pt = ROOK;

            if (m.promotion_type() == pt)
                return m;
        } else {
            return m;
        }
    }

    return Move::none();
}

void UCI::parse_position(Position& pos, const std::string& line, StateInfo& si) {
    std::istringstream ss(line);
    std::string token;
    ss >> token;
    ss >> token;

    if (token == "startpos") {
        pos.set_startpos(&si);
        ss >> token;
    } else if (token == "fen") {
        std::string fen;
        int fields = 0;
        while (ss >> token) {
            if (token == "moves")
                break;
            if (fields++)
                fen += ' ';
            fen += token;
            if (fields >= 6)
                break;
        }
        pos.set(fen, &si);
    }

    if (token == "moves") {
        static StateInfo move_history[1024];
        int history_idx = 0;
        while (ss >> token && history_idx < 1023) {
            const Move m = parse_move(pos, token);
            if (m != Move::none())
                pos.do_move(m, move_history[history_idx++]);
        }
    }
}

void UCI::parse_go(Position& pos, const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    ss >> token;

    SearchLimits limits;
    while (ss >> token) {
        if (token == "wtime") ss >> limits.wtime;
        else if (token == "btime") ss >> limits.btime;
        else if (token == "winc") ss >> limits.winc;
        else if (token == "binc") ss >> limits.binc;
        else if (token == "movestogo") ss >> limits.movestogo;
        else if (token == "depth") ss >> limits.depth;
        else if (token == "nodes") ss >> limits.nodes;
        else if (token == "mate") ss >> limits.mate;
        else if (token == "movetime") ss >> limits.movetime;
        else if (token == "infinite") limits.infinite = true;
    }

    GlobalSearcher.start_search(pos, limits);
}

void UCI::process_command(const std::string& line) {
    initialize();
    g_output.clear();

    std::istringstream ss(line);
    std::string command;
    ss >> command;

    if (command == "uci") {
        emit("id name Andromeda");
        emit("id author Andromeda Dev Team");
        emit("option name Hash type spin default 64 min 1 max 4096");
        emit("option name Aggression type spin default 55 min 0 max 100");
        emit("option name Threads type spin default 1 min 1 max 1");
        emit("uciok");
    } else if (command == "isready") {
        emit("readyok");
    } else if (command == "setoption") {
        std::string rest;
        std::getline(ss, rest);
        const std::size_t value_pos = rest.find(" value ");
        std::string name = value_pos == std::string::npos ? rest : rest.substr(0, value_pos);
        std::string value = value_pos == std::string::npos ? "" : rest.substr(value_pos + 7);

        while (!name.empty() && name.front() == ' ') name.erase(name.begin());
        while (!name.empty() && name.back() == ' ') name.pop_back();

        if (name == "Hash" && !value.empty()) {
            TT.resize(static_cast<size_t>(std::max(1, std::min(4096, std::stoi(value)))));
        } else if (name == "Aggression" && !value.empty()) {
            set_aggression(std::stoi(value));
        }
    } else if (command == "ucinewgame") {
        TT.clear();
        g_position.set_startpos(&g_root_state);
    } else if (command == "position") {
        parse_position(g_position, line, g_root_state);
    } else if (command == "go") {
        parse_go(g_position, line);
    } else if (command == "stop") {
        GlobalSearcher.stop();
    } else if (command == "eval") {
        emit("info string eval " + std::to_string(evaluate(g_position)));
    } else if (command == "d") {
        emit("info string fen " + g_position.fen());
    }
}

void UCI::loop() {
    initialize();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit")
            break;

        process_command(line);
        std::cout << g_output;
        std::cout.flush();
    }
}

} // namespace Andromeda

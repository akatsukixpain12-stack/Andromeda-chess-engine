#include "uci.h"
#include "tt.h"
#include "attacks.h"
#include "evaluate.h"
#include <iostream>
#include <sstream>
#include <thread>

namespace Andromeda {

Move UCI::parse_move(const Position& pos, const std::string& str) {
    if (str.length() < 4) return Move::none();

    Square from = make_square(static_cast<File>(str[0] - 'a'), static_cast<Rank>(str[1] - '1'));
    Square to = make_square(static_cast<File>(str[2] - 'a'), static_cast<Rank>(str[3] - '1'));

    Move legal[256];
    Position temp_pos = pos;
    int count = generate_legal_moves(temp_pos, legal);

    for (int i = 0; i < count; ++i) {
        Move m = legal[i];
        if (m.from() == from && m.to() == to) {
            if (m.type() == PROMOTION) {
                if (str.length() >= 5) {
                    char promo = std::tolower(str[4]);
                    PieceType pt = QUEEN;
                    if (promo == 'n') pt = KNIGHT;
                    else if (promo == 'b') pt = BISHOP;
                    else if (promo == 'r') pt = ROOK;
                    if (m.promotion_type() == pt) return m;
                }
            } else {
                return m;
            }
        }
    }
    return Move::none();
}

void UCI::parse_position(Position& pos, const std::string& line, StateInfo& si) {
    std::istringstream ss(line);
    std::string token;
    ss >> token; // skip "position"

    ss >> token;
    if (token == "startpos") {
        pos.set_startpos(&si);
        ss >> token; // check if "moves" follows
    } else if (token == "fen") {
        std::string fen;
        while (ss >> token && token != "moves") {
            fen += token + " ";
        }
        pos.set(fen, &si);
    }

    if (token == "moves") {
        static StateInfo move_history[1024];
        int history_idx = 0;
        while (ss >> token) {
            Move m = parse_move(pos, token);
            if (m != Move::none()) {
                pos.do_move(m, move_history[history_idx++]);
            }
        }
    }
}

void UCI::parse_go(Position& pos, const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    ss >> token; // skip "go"

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

void UCI::loop() {
    Position pos;
    StateInfo si;
    pos.set_startpos(&si);

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "uci") {
            std::cout << "id name Andromeda 3.0" << std::endl;
            std::cout << "id author Andromeda Dev Team" << std::endl;
            std::cout << "option name Hash type spin default 64 min 1 max 65536" << std::endl;
            std::cout << "option name Threads type spin default 1 min 1 max 512" << std::endl;
            std::cout << "option name OwnBook type check default true" << std::endl;
            std::cout << "option name Skill Level type spin default 20 min 0 max 20" << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (command == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (command == "setoption") {
            std::string name_token, name, value_token, value;
            ss >> name_token >> name >> value_token >> value;
            if (name == "Hash") {
                TT.resize(std::stoi(value));
            }
        } else if (command == "ucinewgame") {
            TT.clear();
            pos.set_startpos(&si);
        } else if (command == "position") {
            parse_position(pos, line, si);
        } else if (command == "go") {
            parse_go(pos, line);
        } else if (command == "stop") {
            GlobalSearcher.stop();
        } else if (command == "eval") {
            std::cout << "Evaluation: " << evaluate(pos) << " cp" << std::endl;
        } else if (command == "d") {
            std::cout << pos.fen() << std::endl;
        } else if (command == "quit") {
            break;
        }
    }
}

} // namespace Andromeda

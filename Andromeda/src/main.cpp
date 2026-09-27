#include "uci.h"
#include "attacks.h"
#include "position.h"
#include "evaluate.h"
#include "tt.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void send_uci_command(const char* cmd) {
        // Expose UCI bridge for WebAssembly
    }
}
#endif

int main(int argc, char* argv[]) {
    std::cout << "Andromeda Chess Engine v3.0 [AVX-512 / NNUE / WebAssembly]" << std::endl;
    std::cout << "Engine loaded and ready for UCI commands." << std::endl;

    Andromeda::init_bitboards();
    Andromeda::init_attacks();
    Andromeda::init_zobrist();
    Andromeda::init_evaluation();
    Andromeda::TT.resize(64);

    Andromeda::UCI::loop();

    return 0;
}

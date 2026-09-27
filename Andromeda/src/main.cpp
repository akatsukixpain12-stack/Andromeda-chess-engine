#include "uci.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
extern "C" {
EMSCRIPTEN_KEEPALIVE
void initialize_engine() {
    Andromeda::UCI::initialize();
}
EMSCRIPTEN_KEEPALIVE
void send_uci_command(const char* cmd) {
    if (cmd)
        Andromeda::UCI::process_command(cmd);
}
EMSCRIPTEN_KEEPALIVE
const char* get_uci_output() {
    return Andromeda::UCI::output_c_str();
}
}
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    Andromeda::UCI::initialize();
#ifndef __EMSCRIPTEN__
    Andromeda::UCI::loop();
#endif
    return 0;
}

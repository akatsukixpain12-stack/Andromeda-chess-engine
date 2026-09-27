// Browser convenience API appended to the generated Emscripten module.
// The engine itself remains UCI-compatible; this only makes embedding easier.
Module["createAndromedaEngine"] = function() {
    if (typeof Module["_initialize_engine"] === "function") {
        Module["_initialize_engine"]();
    }
    return {
        uci(command) {
            Module["ccall"]("send_uci_command", null, ["string"], [String(command)]);
            return Module["UTF8ToString"](Module["_get_uci_output"]());
        },
        output() {
            return Module["UTF8ToString"](Module["_get_uci_output"]());
        },
        initialize() {
            Module["_initialize_engine"]();
        }
    };
};

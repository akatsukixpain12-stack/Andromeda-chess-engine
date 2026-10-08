/**
 * Andromeda browser integration.
 *
 * The generated dist/andromeda.js is an ESM factory produced by Emscripten.
 * Copy this file into your website (or adjust the import path).
 *
 * Important: the current native search API is synchronous. Calling go()
 * performs the search on the calling thread. For a UI that must stay
 * responsive, run this wrapper inside a Web Worker.
 */

type EmscriptenModule = {
    _initialize_engine: () => void;
    _send_uci_command: (command: string) => void;
    _get_uci_output: () => number;
    UTF8ToString: (ptr: number) => string;
};

type AndromedaFactory = (options?: {
    locateFile?: (file: string, prefix: string) => string;
}) => Promise<EmscriptenModule>;

export type AndromedaEngine = {
    uci: (command: string) => string;
    newGame: () => string;
    setOption: (name: string, value: string | number) => string;
    positionStart: (moves?: string[]) => string;
    positionFen: (fen: string, moves?: string[]) => string;
    go: (options?: {
        depth?: number;
        movetime?: number;
        nodes?: number;
        infinite?: boolean;
    }) => string;
    stop: () => string;
    evaluate: () => string;
    debug: () => string;
};

export async function createAndromedaEngine(
    wasmBaseUrl = new URL("../dist/", import.meta.url),
): Promise<AndromedaEngine> {
    // Emscripten emits the generated file as an ES module with a default
    // factory when MODULARIZE + EXPORT_ES6 are enabled.
    // @ts-ignore The generated Emscripten file has no hand-written .d.ts file.
    const imported = await import("../dist/andromeda.js");
    const factory = imported.default as AndromedaFactory;

    const module = await factory({
        locateFile(file: string, prefix: string) {
            if (file.endsWith(".wasm"))
                return new URL(file, wasmBaseUrl).href;
            return prefix + file;
        },
    });

    module._initialize_engine();

    const uci = (command: string): string => {
        module._send_uci_command(command);
        return module.UTF8ToString(module._get_uci_output());
    };

    return {
        uci,
        newGame: () => uci("ucinewgame"),
        setOption: (name, value) => uci(`setoption name ${name} value ${value}`),
        positionStart: (moves = []) =>
            uci(moves.length ? `position startpos moves ${moves.join(" ")}` : "position startpos"),
        positionFen: (fen, moves = []) =>
            uci(`position fen ${fen}${moves.length ? ` moves ${moves.join(" ")}` : ""}`),
        go: (options = {}) => {
            const parts: string[] = [];
            if (options.depth !== undefined) parts.push(`depth ${options.depth}`);
            if (options.movetime !== undefined) parts.push(`movetime ${options.movetime}`);
            if (options.nodes !== undefined) parts.push(`nodes ${options.nodes}`);
            if (options.infinite) parts.push("infinite");
            return uci(`go${parts.length ? " " + parts.join(" ") : ""}`);
        },
        stop: () => uci("stop"),
        evaluate: () => uci("eval"),
        debug: () => uci("d"),
    };
}

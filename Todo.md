# Zen++ Roadmap

## 1. Interpreter Optimizations
- [x] Flag-based control flow (replace throw/catch for return/break/continue) — 17x faster function calls
- [x] Arena allocator for AST nodes — contiguous 4096-node blocks, ~0 malloc in hot path
- [ ] Constant folding at parse time
- [ ] Pre-allocate scope frames
- [ ] WASM bundle size optimization (-Oz + compression)
- [ ] Lazy WASM loading
- [ ] Avoid unnecessary Value copies
- [ ] Index-based variable lookup

## 2. C++ Transpiler
Transpile zen++ to C++ so you can submit to Codeforces/AtCoder at full C++ speed.

- [ ] `zenpp --emit-cpp solution.zpp > solution.cpp` CLI flag
- [ ] AST walker that outputs C++ strings instead of evaluating
- [ ] Map zen++ constructs to C++ equivalents
- [ ] Type inference for generated C++ (int64_t, double, string, vector)
- [ ] "Generate C++" button in web IDE
- [ ] Two-step pipeline: zen++ → C++ → native binary via gcc/clang
- [ ] Auto-compile mode: `zenpp --compile solution.zpp -o solution` (transpile + gcc in one step)

The interpreter stays for the REPL and web IDE (instant feedback). The transpiler is for contest submissions (max speed). Zen++ becomes a multi-paradigm language — interpreted for development, compiled for production.

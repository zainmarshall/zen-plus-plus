# ZEN++ Web Demo

This folder hosts a minimal WebAssembly demo for the ZEN++ interpreter.

## Build (Emscripten required)

1. Install Emscripten and ensure `emcc` is on PATH.
2. From the repo root:

```bash
emcc -std=c++17 -O2 \
  src/main.cpp src/lexer.cpp src/parser.cpp \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=Zenpp \
  -s EXPORTED_FUNCTIONS='["_zenpp_eval","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["cwrap","UTF8ToString"]' \
  -o web/zenpp.js
```

## Run locally

Any static file server works:

```bash
cd web
python3 -m http.server 8080
```

Open `http://localhost:8080`.

## Overview

This repository is a small C++20 Sudoku solver that builds as a native executable and as a WebAssembly module (via Emscripten).

- Top-level build: `CMakeLists.txt` (C++20, exports compile_commands.json).
- Source layout: headers in `include/` and implementation in `src/`.
- Web artifacts are copied to `web/public/wasm/` by a CMake post-build step.

## Big picture / architecture

- Single binary target named `sudoku-engine` created from all `src/*.cpp` and `include/*` (see `CMakeLists.txt`).
- Native build: produces an executable `./sudoku-engine`.
- WASM build (when using Emscripten): produces `sudoku-engine.js` and `sudoku-engine.wasm`, exported as an ES6 module (`createSudokuModule`) and copied to `web/public/wasm/`.
- Example runtime entry: `src/main.cpp` — it constructs `SudokuSolver` and calls `init()`.

## Build & developer workflows (exact commands)

- Native (out-of-source):
  - mkdir build && cd build
  - cmake ..
  - cmake --build .
  - ./sudoku-engine

- Release:
  - cmake -DCMAKE_BUILD_TYPE=Release ..
  - cmake --build .

- WebAssembly (Emscripten must be installed and activated):
  - mkdir build-wasm && cd build-wasm
  - emcmake cmake ..
  - cmake --build .
  - Artifacts will be copied to `web/public/wasm/` by the post-build rule in `CMakeLists.txt`.

## Project-specific conventions & patterns

- Header files live in `include/engine/` and are included with quoted paths: `#include "engine/SudokuSolver.h"` (see `src/main.cpp`).
- Implementation files live under `src/engine/` and are globbed by CMake (`file(GLOB_RECURSE SOURCES "src/*.cpp")`).
- CMake defines the `EMSCRIPTEN` symbol for WASM builds — use `if(EMSCRIPTEN)` branch in CMake to change WASM-specific behavior.
- The CMake file sets `CMAKE_EXPORT_COMPILE_COMMANDS ON` — use `compile_commands.json` for static analysis or language servers.

## Integration points & examples

- WASM frontend expects an ES module at `/wasm/sudoku-engine.js`. Example import (from `README.md`):

```js
import createSudokuModule from "/wasm/sudoku-engine.js"
const Module = await createSudokuModule()
```

- If you add new public APIs intended for WASM exposure, leave the CMake `EMSCRIPTEN` entry points intact; the project uses `-sEXPORT_ES6=1 -sMODULARIZE=1 -sEXPORT_NAME=createSudokuModule`.

## Editing guidance for AI agents

- When adding a new class, put the header in `include/engine/` and implementation in `src/engine/`. Update nothing in `CMakeLists.txt` because it already globs sources.
- To change how WASM artifacts are placed, update the post-build `add_custom_command` in `CMakeLists.txt` (currently copies `${PROJECT_NAME}.js` and `.wasm` to `web/public/wasm/`).
- Preserve the project target name (`sudoku-engine`) unless intentionally renaming—CMake uses it repeatedly in properties and copy steps.

## Files to check first when investigating behavior

- `CMakeLists.txt` — build modes, WASM flags, post-build copy
- `src/main.cpp` — example usage and runtime entry
- `include/engine/SudokuSolver.h` and `src/engine/SudokuSolver.cpp` — core solver API and patterns
- `web/public/wasm/README.md` — notes for frontend consumers of WASM artifacts

## Quick pitfalls & notes

- There are no unit tests or test harness in the repo. Be conservative when proposing test scaffolding; mention preferred test framework choices before adding.
- Changes that affect the WASM build should be validated with `emcmake`/Emscripten and by checking `web/public/wasm/` outputs.

---

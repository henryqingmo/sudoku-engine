#ifdef __EMSCRIPTEN__

#include <memory>

#include "engine/solver.h"

using sudoku_engine::Board;
using sudoku_engine::BoardCell;
using sudoku_engine::BoardOffset;

using sudoku_engine::Solver;

// Global instances for WASM
static std::unique_ptr<Solver> solver = nullptr;
static std::unique_ptr<Board> board;

extern "C" {
// Initialize the solver
void initGame() {
    solver = std::make_unique<Solver>();
}

// Solve the puzzle
bool runSolver() {
    // TODO: Later allow users to step through the process etc.
    if (!solver)
        return false;
    return solver->solve(*board);
}

// Get board value at position (0-based indexing)
BoardCell getBoardValue(BoardOffset row, BoardOffset col) {
    if (!board)
        return 0;
    if (row < 0 || row >= 9 || col < 0 || col >= 9)
        return 0;
    return board->getValues()[{row, col}];
}

// Cleanup
void cleanupSolver() {
    if (solver) {
        solver = nullptr;
    }
}
}

#endif

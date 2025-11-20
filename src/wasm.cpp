#ifdef __EMSCRIPTEN__

#include <memory>

#include "engine/solver.h"
#include "heuristic/backtrack.h"
#include "heuristic/forward.h"

using sudoku_engine::Board;
using sudoku_engine::BoardCage;
using sudoku_engine::BoardCell;
using sudoku_engine::BoardOffset;

using sudoku_engine::BacktrackHeuristic;
using sudoku_engine::ForwardHeuristic;
using sudoku_engine::Solver;

// Global instances for WASM
static Solver solver;
static Board board;
static std::unique_ptr<BacktrackHeuristic> heuristic = nullptr;

static const BoardCage SAMPLE_CAGES[] = {
    {6, {{5, 1}}},
    {7, {{8, 4}}},
    {13, {{0, 0}, {1, 0}}},
    {14, {{2, 0}, {2, 1}}},
    {3, {{0, 1}, {1, 1}}},
    {4, {{3, 0}, {3, 1}}},
    {14, {{4, 0}, {4, 1}}},
    {15, {{5, 0}, {6, 0}}},
    {5, {{7, 0}, {8, 0}}},
    {8, {{3, 2}, {3, 3}}},
    {15, {{4, 2}, {4, 3}}},
    {10, {{6, 2}, {6, 3}}},
    {4, {{1, 3}, {2, 3}}},
    {16, {{2, 4}, {2, 5}}},
    {13, {{3, 4}, {3, 5}}},
    {7, {{6, 4}, {7, 4}}},
    {7, {{6, 5}, {7, 5}}},
    {9, {{0, 8}, {1, 8}}},
    {10, {{6, 7}, {7, 7}}},
    {6, {{6, 8}, {7, 8}}},
    {17, {{8, 7}, {8, 8}}},
    {15, {{0, 2}, {1, 2}, {2, 2}}},
    {18, {{8, 2}, {7, 3}, {8, 3}}},
    {15, {{4, 5}, {5, 5}, {4, 6}}},
    {12, {{2, 6}, {2, 7}, {1, 7}}},
    {14, {{3, 6}, {3, 7}, {4, 7}}},
    {16, {{5, 6}, {5, 7}, {5, 8}}},
    {18, {{2, 8}, {3, 8}, {4, 8}}},
    {25, {{6, 1}, {7, 1}, {8, 1}, {7, 2}}},
    {10, {{5, 2}, {5, 3}, {5, 4}, {4, 4}}},
    {17, {{0, 3}, {0, 4}, {1, 4}, {1, 5}}},
    {15, {{8, 5}, {8, 6}, {7, 6}, {6, 6}}},
    {27, {{0, 5}, {0, 6}, {0, 7}, {1, 6}}}
};

extern "C" {
// Initialize the solver
void initGame(bool forward, bool mrv, bool lcv) {
    constexpr std::size_t STEP_LIMIT = 100000000;
    board = Board();
    board.setCages(SAMPLE_CAGES);
    heuristic =
        forward ?
            std::make_unique<ForwardHeuristic>(board, STEP_LIMIT, mrv, lcv) :
            std::make_unique<BacktrackHeuristic>(board, STEP_LIMIT);
}

// Solve the puzzle
bool runSolver() {
    // TODO: Later allow users to step through the process etc.
    if (!heuristic)
        return false;

    return solver.solve(*heuristic);
}

// Get board value at position (0-based indexing)
BoardCell getBoardValue(BoardOffset row, BoardOffset col) {
    if (row < 0 || row >= 9 || col < 0 || col >= 9)
        return 0;
    return board.getValues()[{row, col}];
}
}

#endif

#pragma once

#include <utility>
#include <vector>

#include "board.h"

SUDOKU_NAMESPACE {
    class Solver {
    public:
        Solver();
        ~Solver();

        bool solve(Board& board);

    private:
        // Backtracking solver
        bool solveBacktrack(Board& board, BoardPosition pos);
    };
}

#include <iostream>
#include <numeric>

#include "engine/solver.h"

using sudoku_engine::Solver;

Solver::Solver() {
    std::cout << "Killer Sudoku solver initialized" << std::endl;
}

Solver::~Solver() {
    // Destructor
}

bool Solver::solve(Board& board) {
    return solveBacktrack(board, {0, 0});
}

bool Solver::solveBacktrack(Board& board, BoardPosition pos) {
    // Move to next row if we've reached the end of current row
    if (pos.col == BOARD_SIZE) {
        pos.row++;
        pos.col = 0;
    }

    // If we've filled all rows, we're done
    if (pos.row == BOARD_SIZE) {
        return true;
    }

    auto& cell_values = board.getValues();

    // Skip cells that are already filled
    if (cell_values[pos] != CELL_EMPTY) {
        return solveBacktrack(board, BoardPosition(pos.row, pos.col + 1));
    }

    // Try placing numbers 1-9
    for (BoardCell num = CELL_MIN; num <= CELL_MAX; num++) {
        cell_values[pos] = num;

        if (board.isInvalid(pos)) {
            continue;
        }

        // Recursively try to fill the rest
        if (solveBacktrack(board, BoardPosition(pos.row, pos.col + 1))) {
            return true;
        }
    }

    // Backtrack if this placement doesn't lead to a solution
    cell_values[pos] = CELL_EMPTY;

    return false;
}

#include "heuristic/backtrack.h"

using sudoku_engine::BacktrackHeuristic;

bool BacktrackHeuristic::solve() {
    return this->expand({0, 0});
}

bool BacktrackHeuristic::expand(const BoardPosition& pos) {
    // If we've filled all rows, we're done
    if (pos.row == BOARD_SIZE) {
        return true;
    }

    this->step_count++;
    this->checkStepLimit();

    auto& cell_value = this->board.getValues()[pos];

    // Skip cells that are already filled
    if (cell_value != CELL_EMPTY) {
        return this->expand(this->incrementPos(pos));
    }

    // Try placing numbers 1-9
    for (BoardCell num = CELL_MIN; num <= CELL_MAX; num++) {
        cell_value = num;

        if (this->board.isInvalid(pos)) {
            cell_value = CELL_EMPTY;
            continue;
        }
        
        if (this->expand(this->incrementPos(pos))) {
            return true;
        }
    }

    cell_value = CELL_EMPTY;
    return false;
}

#include "heuristic/backtrack.h"

using sudoku_engine::BacktrackHeuristic;

bool BacktrackHeuristic::solve() {
    return this->solve({0, 0});
}

bool BacktrackHeuristic::solve(BoardPosition pos) {
    // Move to next row if we've reached the end of current row
    if (pos.col == BOARD_SIZE) {
        pos.row++;
        pos.col = 0;
    }

    // If we've filled all rows, we're done
    if (pos.row == BOARD_SIZE) {
        return true;
    }

    this->step_count++;

    auto& cell_values = this->board.getValues();

    // Skip cells that are already filled
    if (cell_values[pos] != CELL_EMPTY) {
        return this->solve(BoardPosition(pos.row, pos.col + 1));
    }

    const auto invalid_choices = this->getInvalidChoices(pos);
    if ((-invalid_choices).empty()) {
        return false;
    }

    // Try placing numbers 1-9
    for (BoardCell num = CELL_MIN; num <= CELL_MAX; num++) {
        cell_values[pos] = num;

        if (invalid_choices.has(num) || this->board.isInvalid(pos)) {
            cell_values[pos] = CELL_EMPTY;
            continue;
        }

        if (this->onUpdate(pos)) {
            // Recursively try to fill the rest if legal
            if (this->solve(BoardPosition(pos.row, pos.col + 1))) {
                return true;
            }
        }

        // Backtrack if this placement doesn't lead to a solution
        cell_values[pos] = CELL_EMPTY;
        this->onUpdate(pos);
    }

    return false;
}

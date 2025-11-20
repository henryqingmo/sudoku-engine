#include "heuristic/sbacktrack.h"

using sudoku_engine::SBacktrackHeuristic;

SBacktrackHeuristic::StepResult SBacktrackHeuristic::step() {
    if (!stepping_initialized) {
        startStepping();
    }

    if (stack.empty()) {
        return StepResult::STEP_FAIL; // no solution
    }

    Frame &f = stack.back();
    const BoardPosition pos = f.pos;

    // success condition
    if (pos.row == BOARD_SIZE) {
        return StepResult::STEP_SUCC;
    }

    this->step_count++;
    this->checkStepLimit();

    auto &cell_value = this->board.getValues()[pos];

    // First time we enter this frame
    if (f.next_value == 0) {
        f.next_value = 1; // CELL_MIN
        f.skip_cell = (cell_value != CELL_EMPTY);
        if (f.skip_cell) {
            // Skip fixed cell: emulate recursive "expand(incrementPos(pos))"
            stack.push_back(Frame{ this->incrementPos(pos), 0, false });
            return StepResult::STEP_MORE;
        }
        cell_value = CELL_EMPTY;
    }

    // If this frame is for a filled cell, we never try candidates
    if (f.skip_cell) {
        return StepResult::STEP_MORE;
    }

    // Try candidates sequentially
    while (f.next_value <= CELL_MAX) {
        cell_value = static_cast<BoardCell>(f.next_value);

        if (!this->board.isInvalid(pos)) {
            // Push next position (emulate recursive call)
            stack.push_back(Frame{ this->incrementPos(pos), 0, false });
            f.next_value++;   // next time, continue after this child
            return StepResult::STEP_MORE;
        }

        // invalid → undo and try next
        cell_value = CELL_EMPTY;
        f.next_value++;
    }

    // Exhausted all values → backtrack
    cell_value = CELL_EMPTY;
    stack.pop_back();
    return StepResult::STEP_MORE;
}

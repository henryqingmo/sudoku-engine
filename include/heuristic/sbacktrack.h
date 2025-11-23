#pragma once

#include <stdexcept>

#include "backtrack.h"

SUDOKU_NAMESPACE {
    class SBacktrackHeuristic : public virtual BacktrackHeuristic {
    protected:
        enum class StepResult : short {
            STEP_MORE = -1,
            STEP_FAIL = 0,
            STEP_SUCC = 1
        };

        struct Frame {
            BoardPosition pos;
            int next_value;
            bool skip_cell;
        };

        std::vector<Frame> stack;
        bool stepping_initialized = false;

    public:
        using BacktrackHeuristic::BacktrackHeuristic;

        virtual StepResult step();

        virtual BoardCellDomain getDomain(const BoardPosition& pos) const {
            return ~BoardCellDomain();
        }

    protected:
        void startStepping() {
            this->stack.clear();
            this->stepping_initialized = true;
            stack.push_back(Frame{{0, 0}, 0, false});
        }
    };
}

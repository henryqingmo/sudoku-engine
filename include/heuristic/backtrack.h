#pragma once

#include "heuristic.h"

SUDOKU_NAMESPACE {
    class BacktrackHeuristic : public Heuristic {
    private:
        std::size_t step_count = 0;

    public:
        BacktrackHeuristic(Board& board) : Heuristic(board) {}

        bool solve() final;

        std::size_t getStepCount() const {
            return this->step_count;
        }

    protected:
        virtual BoardCellDomain getInvalidChoices(const BoardPosition&) const {
            return BoardCellDomain();
        }

        virtual bool onUpdate(const BoardPosition&) {
            return true;
        }

    private:
        bool solve(BoardPosition pos);
    };
}

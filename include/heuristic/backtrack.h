#pragma once

#include "heuristic.h"

SUDOKU_NAMESPACE {
    class BacktrackHeuristic : public Heuristic {
    protected:
        std::size_t step_count = 0;

    public:
        BacktrackHeuristic(Board& board) : Heuristic(board) {}

        bool solve() override;

        std::size_t getStepCount() const {
            return this->step_count;
        }

    protected:
        static constexpr BoardPosition incrementPos(const BoardPosition& pos) {
            auto next = BoardPosition(pos.row, pos.col + 1);
            if (next.col == BOARD_SIZE) {
                next.row++;
                next.col = 0;
            }
            return next;
        }

        virtual bool expand(const BoardPosition& pos);
    };
}

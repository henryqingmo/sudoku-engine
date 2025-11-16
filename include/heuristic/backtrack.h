#pragma once

#include <stdexcept>

#include "heuristic.h"

SUDOKU_NAMESPACE {
    class BacktrackHeuristic : public Heuristic {
    public:
        class TooHardError : public std::runtime_error {
            using runtime_error::runtime_error;
        };

    protected:
        std::size_t step_count = 0;
        std::size_t step_limit;

    public:
        BacktrackHeuristic(Board& board, std::size_t step_limit)
            : Heuristic(board), step_limit(step_limit) {}

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

        void checkStepLimit() {
            if (this->step_count > this->step_limit) {
                throw TooHardError("Rage quit");
            }
        }
    };
}

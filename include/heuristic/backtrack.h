#pragma once

#include "heuristic.h"

SUDOKU_NAMESPACE {
    class BacktrackHeuristic : public Heuristic {
    public:
        BacktrackHeuristic(Board& board) : Heuristic(board) {}

        bool solve() final;

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

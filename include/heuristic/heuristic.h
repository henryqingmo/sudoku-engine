#pragma once

#include "../engine/board.h"

SUDOKU_NAMESPACE {
    class Heuristic {
    protected:
        Board& board;

    public:
        explicit Heuristic(Board& board) : board(board) {}

        virtual bool solve() = 0;

        virtual ~Heuristic() = default;
    };
}

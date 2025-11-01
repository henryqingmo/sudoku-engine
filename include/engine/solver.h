#pragma once

#include <utility>
#include <vector>

#include "../heuristic/heuristic.h"

SUDOKU_NAMESPACE {
    class Solver {
    public:
        Solver();
        ~Solver();

        bool solve(Heuristic& heuristic);
    };
}

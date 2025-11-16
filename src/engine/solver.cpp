#include <iostream>
#include <numeric>

#include "engine/solver.h"

using sudoku_engine::Solver;

Solver::Solver() = default;

Solver::~Solver() = default;

bool Solver::solve(Heuristic& heuristic) {
    return heuristic.solve();
}

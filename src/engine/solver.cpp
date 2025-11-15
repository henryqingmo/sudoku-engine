#include <iostream>
#include <numeric>

#include "engine/solver.h"

using sudoku_engine::Solver;

Solver::Solver() {
    std::cout << "Killer Sudoku solver initialized" << std::endl;
}

Solver::~Solver() = default;

bool Solver::solve(Heuristic& heuristic) {
    return heuristic.solve();
}

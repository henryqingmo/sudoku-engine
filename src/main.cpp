#include <iostream>
#include <chrono>
#include "engine/solver.h"
#include "heuristic/backtrack.h"
#include "heuristic/forward.h"

int main() {
    using sudoku_engine::Solver;

    using sudoku_engine::Board;
    using sudoku_engine::BoardCage;

    using sudoku_engine::BacktrackHeuristic;
    using sudoku_engine::ForwardHeuristic;

    std::cout << "Killer Sudoku Solver v0.1.0" << std::endl;
    std::cout << "===========================" << std::endl;

    Solver solver;
    Board board;

    // Killer sudoku puzzle configuration
    // Define cages with their target sums and cell coordinates (row, col)
    std::vector<BoardCage> cages = {
        {6, {{5, 1}}},
        {7, {{8, 4}}},
        {13, {{0, 0}, {1, 0}}},
        {14, {{2, 0}, {2, 1}}},
        {3, {{0, 1}, {1, 1}}},
        {4, {{3, 0}, {3, 1}}},
        {14, {{4, 0}, {4, 1}}},
        {15, {{5, 0}, {6, 0}}},
        {5, {{7, 0}, {8, 0}}},
        {8, {{3, 2}, {3, 3}}},
        {15, {{4, 2}, {4, 3}}},
        {10, {{6, 2}, {6, 3}}},
        {4, {{1, 3}, {2, 3}}},
        {16, {{2, 4}, {2, 5}}},
        {13, {{3, 4}, {3, 5}}},
        {7, {{6, 4}, {7, 4}}},
        {7, {{6, 5}, {7, 5}}},
        {9, {{0, 8}, {1, 8}}},
        {10, {{6, 7}, {7, 7}}},
        {6, {{6, 8}, {7, 8}}},
        {17, {{8, 7}, {8, 8}}},
        {15, {{0, 2}, {1, 2}, {2, 2}}},
        {18, {{8, 2}, {7, 3}, {8, 3}}},
        {15, {{4, 5}, {5, 5}, {4, 6}}},
        {12, {{2, 6}, {2, 7}, {1, 7}}},
        {14, {{3, 6}, {3, 7}, {4, 7}}},
        {16, {{5, 6}, {5, 7}, {5, 8}}},
        {18, {{2, 8}, {3, 8}, {4, 8}}},
        {25, {{6, 1}, {7, 1}, {8, 1}, {7, 2}}},
        {10, {{5, 2}, {5, 3}, {5, 4}, {4, 4}}},
        {17, {{0, 3}, {0, 4}, {1, 4}, {1, 5}}},
        {15, {{8, 5}, {8, 6}, {7, 6}, {6, 6}}},
        {27, {{0, 5}, {0, 6}, {0, 7}, {1, 6}}}
    };

    // Set up the puzzle
    board.setCages(cages);

    // Optional: start with some given numbers (empty board for this example)
    // ...

    std::cout << std::endl << "Initial Board:" << std::endl;
    board.print(std::cout);

    std::cout << std::endl << "Solving..." << std::endl;

    // Solve the puzzle
    auto heuristic = ForwardHeuristic(board); // BacktrackHeuristic(board);
    auto start = std::chrono::high_resolution_clock::now();
    bool solved = solver.solve(heuristic);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    if (solved) {
        std::cout << std::endl << "[DONE] Solution found!" << std::endl;
        board.print(std::cout);
    } else {
        std::cout << std::endl << "[FAIL] No solution exists for this puzzle." << std::endl;
        board.print(std::cout);
    }
    std::cout << std::endl << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}

#include "engine/SudokuSolver.h"
#include <iostream>

// Global solver instance for WASM
static SudokuSolver* globalSolver = nullptr;

#ifdef __EMSCRIPTEN__
extern "C" {
    // Initialize the solver
    void initSolver() {
        if (globalSolver) delete globalSolver;
        globalSolver = new SudokuSolver();
        globalSolver->init();
    }

    // Solve the puzzle
    bool solvePuzzle() {
        if (!globalSolver) return false;
        return globalSolver->solve();
    }

    // Get board value at position (0-based indexing)
    int getBoardValue(int row, int col) {
        if (!globalSolver) return 0;
        auto board = globalSolver->getBoard();
        if (row < 0 || row >= 9 || col < 0 || col >= 9) return 0;
        return board[row][col];
    }

    // Cleanup
    void cleanupSolver() {
        if (globalSolver) {
            delete globalSolver;
            globalSolver = nullptr;
        }
    }
}
#endif

int main() {
    std::cout << "Killer Sudoku Solver v0.1.0" << std::endl;
    std::cout << "===========================" << std::endl;
    
    SudokuSolver solver;
    solver.init();
    
    // Killer sudoku puzzle configuration
    // Define cages with their target sums and cell coordinates (row, col)
    std::vector<Cage> cages = {
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
    solver.setCages(cages);
    
    // Optional: start with some given numbers (empty board for this example)
    std::vector<std::vector<int>> initialBoard(9, std::vector<int>(9, 0));
    solver.setBoard(initialBoard);
    
    std::cout << "\nInitial Board:" << std::endl;
    solver.printBoard();
    
    std::cout << "\nSolving..." << std::endl;
    
    // Solve the puzzle
    if (solver.solve()) {
        std::cout << "\n✓ Solution found!" << std::endl;
        solver.printBoard();
    } else {
        std::cout << "\n✗ No solution exists for this puzzle." << std::endl;
    }
    
    return 0;
}

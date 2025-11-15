#include <ctime>
#include <iostream>
#include <string_view>

#include "engine/solver.h"
#include "heuristic/backtrack.h"
#include "heuristic/forward.h"

static inline sudoku_engine::Board makeTestBoard() {
    using sudoku_engine::Board;
    using sudoku_engine::BoardCage;

    // Killer sudoku puzzle configuration
    // Define cages with their target sums and cell coordinates (row, col)
    static const BoardCage cages[] = {
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
    Board board;
    board.setCages(cages);

    // Optional: start with some given numbers (empty board for this example)
    // ...

    return board;
}

int main(int argc, char* argv[]) {
    using sudoku_engine::BacktrackHeuristic;
    using sudoku_engine::Board;
    using sudoku_engine::ForwardHeuristic;
    using sudoku_engine::Heuristic;
    using sudoku_engine::Solver;

    std::cout << "Killer Sudoku Solver v0.1.0" << std::endl;
    std::cout << "===========================" << std::endl;

    const std::string_view arg = (argc > 1) ? argv[1] : "default";

    if (arg == "" || arg == "--help") {
        const std::string_view exe_path = argv[0];
        const std::size_t exe_path_last_sep = exe_path.find_last_of("/\\");
        const std::string_view exe_name =
            exe_path_last_sep == exe_path.npos ?
                exe_path :
                exe_path.substr(exe_path_last_sep + 1);
        std::cout << "Usage: " << exe_name
                  << " [forward [mrv | lcv | mrv lcv] | backtrack]"
                  << std::endl;
        return 1;
    }

    Solver solver;
    Board board = makeTestBoard();

    std::unique_ptr<BacktrackHeuristic> heuristic = nullptr;
    if (arg == "default") {
        heuristic = std::make_unique<ForwardHeuristic>(board, true, true);
    } else if (arg == "forward") {
        using namespace std::string_view_literals;
        const bool mrv = argc > 2 && argv[2] == "mrv"sv;
        const bool lcv = (argc > 2 && argv[2] == "lcv"sv) ||
                         (mrv && argc > 3 && argv[3] == "lcv"sv);
        heuristic = std::make_unique<ForwardHeuristic>(board, mrv, lcv);
    } else if (arg == "backtrack") {
        heuristic = std::make_unique<BacktrackHeuristic>(board);
    } else {
        std::cout << "Invalid heuristic: " << arg << std::endl;
        return 1;
    }

    std::cout << std::endl << "Initial Board:" << std::endl;
    board.print(std::cout);

    std::cout << std::endl << "Solving..." << std::endl;

    // Solve the puzzle
    std::clock_t solving_start = std::clock();
    const bool solution_found = solver.solve(*heuristic);
    std::clock_t solving_end = std::clock();

    if (solution_found) {
        std::cout << std::endl << "[DONE] Solution found!" << std::endl;
        board.print(std::cout);
    } else {
        std::cout << std::endl
                  << "[FAIL] No solution exists for this puzzle." << std::endl;
        board.print(std::cout);
    }

    const auto cpu_time_taken =
        (solving_end - solving_start) / static_cast<double>(CLOCKS_PER_SEC);

    std::cout << std::endl;
    std::cout << "CPU Time Taken: " << cpu_time_taken << " seconds"
              << std::endl;
    std::cout << "Steps Taken:    " << heuristic->getStepCount() << std::endl;

    return 0;
}

#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

#include "engine/solver.h"
#include "heuristic/backtrack.h"
#include "heuristic/forward.h"
#include "serialization.h"

struct Options {
    using HeuristicFactory = std::function<std::unique_ptr<
        sudoku_engine::BacktrackHeuristic>(sudoku_engine::Board& board)>;

    std::unique_ptr<std::ifstream> puzzle_file;
    sudoku_engine::serialization::PuzzleLoader puzzle_loader;
    HeuristicFactory heuristic;
    std::string heuristic_name;
    long puzzle_index;
};

static void printHelp(std::string_view exe_path) {
    const std::size_t exe_path_last_sep = exe_path.find_last_of("/\\");
    const std::string_view exe_name =
        exe_path_last_sep == exe_path.npos ?
            exe_path :
            exe_path.substr(exe_path_last_sep + 1);
    std::cout << "Usage: " << exe_name
              << " [puzzle_bundle_file.ks[:puzzle_index]]"
              << " [step_limit]"
              << " [forward [mrv | lcv | mrv lcv] | backtrack]" << std::endl;
}

static std::unique_ptr<Options> parseOptions(
    const int argc,
    const char* const argv[]
) {
    using sudoku_engine::BacktrackHeuristic;
    using sudoku_engine::Board;
    using sudoku_engine::ForwardHeuristic;
    using sudoku_engine::Heuristic;

    const std::string_view args[] = {
        (argc > 1) ? argv[1] : "",
        (argc > 2) ? argv[2] : "",
        (argc > 3) ? argv[3] : "",
        (argc > 4) ? argv[4] : "",
        (argc > 5) ? argv[5] : "",
    };

    if (args[0] == "" || args[0] == "--help") {
        printHelp(argv[0]);
        return nullptr;
    }

    const std::string_view puzzle_str = args[0];
    const std::string_view step_limit_str = args[1];
    const std::string_view strategy = args[2];

    const std::size_t puzzle_index_pos = puzzle_str.find_last_of(':');
    const std::string filename =
        std::string(puzzle_str.substr(0, puzzle_index_pos));
    const std::string_view puzzle_index_str =
        puzzle_index_pos != std::string_view::npos ?
            puzzle_str.substr(puzzle_index_pos + 1) :
            std::string_view();

    auto puzzle_file =
        std::make_unique<std::ifstream>(filename, std::ios::binary);
    if (!puzzle_file->is_open()) {
        std::cout << "Failed to open file \"" << filename << "\"!";
        return nullptr;
    }

    auto options = std::unique_ptr<Options>(new Options{
        .puzzle_file = nullptr,
        .puzzle_loader =
            sudoku_engine::serialization::PuzzleLoader(*puzzle_file),
        .heuristic = nullptr,
        .heuristic_name = std::string(),
        .puzzle_index = puzzle_index_str.empty() ?
                            -1 :
                            std::stol(std::string(puzzle_index_str))
    });

    options->puzzle_file = std::move(puzzle_file);
    options->heuristic_name = strategy;

    if (step_limit_str.empty()) {
        std::cout << "Step limit required" << std::endl;
        return nullptr;
    }

    const std::size_t step_limit = std::stoull(std::string(step_limit_str));

    using HeuristicPtr = std::unique_ptr<BacktrackHeuristic>;
    if (strategy == "forward") {
        constexpr std::size_t bp = std::size(args) - 2;

        const bool mrv = args[bp] == "mrv";
        const bool lcv = args[bp] == "lcv" || (mrv && args[bp + 1] == "lcv");

        options->heuristic = [=](Board& board) -> HeuristicPtr {
            return std::make_unique<ForwardHeuristic>(
                board, step_limit, mrv, lcv
            );
        };

        if (mrv)
            options->heuristic_name += "-mrv";
        if (lcv)
            options->heuristic_name += "-lcv";
    } else if (strategy == "backtrack") {
        options->heuristic = [step_limit](Board& board) -> HeuristicPtr {
            return std::make_unique<BacktrackHeuristic>(board, step_limit);
        };
    } else {
        std::cout << "Invalid heuristic: \"" << strategy << '"' << std::endl;
        return nullptr;
    }

    return options;
}

static void solvePuzzles(Options& options) {
    using sudoku_engine::BacktrackHeuristic;
    using sudoku_engine::Board;
    using sudoku_engine::Solver;
    using sudoku_engine::serialization::PuzzleLoader;

    Solver solver;

    PuzzleLoader& puzzle_loader = options.puzzle_loader;

    const bool single_puzzle = options.puzzle_index >= 0;

    std::ofstream data_output;

    if (!single_puzzle) {
        const std::time_t result = std::time(nullptr);
        const std::string filename = "experiment-data-" +
                                     options.heuristic_name + '-' +
                                     std::to_string(result) + ".csv";

        data_output.open(filename);
        if (!data_output.is_open()) {
            std::cout << "Could not open \"" << filename << "\" for writing!"
                      << std::endl;
            return;
        } else {
            std::cout << "Writing to \"" << filename << "\"..." << std::endl;
        }

        data_output << "Puzzle,Time,Steps" << std::endl;
    }

    const unsigned long index_start = single_puzzle ? options.puzzle_index : 0;
    const unsigned long index_end =
        single_puzzle ? options.puzzle_index + 1 : puzzle_loader.puzzle_count();
    const unsigned long index_range = index_end - index_start;

    long double total_cpu_time = 0;
    size_t total_steps_taken = 0;
    unsigned long puzzle_count = 0;

    for (unsigned long index = index_start; index < index_end; index++) {
        const auto puzzle = puzzle_loader.load_puzzle(index);
        Board board;

        board.setCages(puzzle->cages.data());

        if (single_puzzle) {
            std::cout << std::endl << "Initial Board:" << std::endl;
            board.print(std::cout);

            std::cout << std::endl << "Solving..." << std::endl;
        }

        const auto heuristic = options.heuristic(board);

        bool solution_found = false;

        // Solve the puzzle
        std::clock_t solving_start = std::clock();
        try {
            solution_found = solver.solve(*heuristic);
        } catch (const BacktrackHeuristic::TooHardError&) {
            std::cout << "  - The solver rage-quit puzzle #" << index << "."
                      << std::endl;
            continue;
        }
        std::clock_t solving_end = std::clock();

        if (solution_found) {
            if (board.getValues() == puzzle->solution) {
                if (single_puzzle) {
                    std::cout << std::endl
                              << "[DONE] Solution found!" << std::endl;
                }
            } else {
                const bool valid = !board.isIncomplete() && !board.isInvalid();

                if (!valid || single_puzzle) {
                    std::cout << std::endl
                              << "[WARN] Solution mismatch!" << std::endl;

                    std::cout << "Received:" << std::endl;
                    board.print(std::cout);

                    std::cout << "Expected:" << std::endl;
                    board.setValues(puzzle->solution);
                    board.print(std::cout);
                }

                if (!valid) {
                    std::cout << "[FAIL] Solution is also invalid!"
                              << std::endl;
                    break;
                } else if (single_puzzle) {
                    std::cout << "[INFO] Alternative solution found."
                              << std::endl;
                }
            }
        } else {
            std::cout << std::endl
                      << "[FAIL] No solution exists for puzzle #" << index
                      << "!" << std::endl;
            board.print(std::cout);
            break;
        }

        if (single_puzzle) {
            board.print(std::cout);
        } else if (puzzle_count % 100 == 0) {
            std::cout << "  > [" << puzzle_count << "/" << index_range << "]"
                      << std::endl;
        }

        const auto cpu_time_taken =
            (solving_end - solving_start) / static_cast<double>(CLOCKS_PER_SEC);
        const auto step_count = heuristic->getStepCount();

        if (!single_puzzle) {
            data_output << index << ',' << cpu_time_taken << ',' << step_count
                        << std::endl;
        }

        total_cpu_time += cpu_time_taken;
        total_steps_taken += step_count;
        puzzle_count++;
    }

    const auto avg_cpu_time = total_cpu_time / puzzle_count;
    const auto avg_step_count =
        total_steps_taken / static_cast<long double>(puzzle_count);

    std::cout << std::endl;
    std::cout << "Puzzles Solved:      " << puzzle_count << " / " << index_range
              << std::endl;
    std::cout << "Avg. CPU Time Taken: " << avg_cpu_time << " seconds"
              << std::endl;
    std::cout << "Avg. Steps Taken:    " << avg_step_count << std::endl;
}

extern "C" {
void initGame(bool forward, bool mrv, bool lcv);
int stepSolver();
}

// void testWasm() {
//     initGame(false, false, false);
//     initGame(false, false, false);
//     stepSolver();
//     initGame(false, false, false);
// }

int main(int argc, char* argv[]) {
    using sudoku_engine::Solver;

    std::cout << "Killer Sudoku Solver v0.1.0" << std::endl;
    std::cout << "===========================" << std::endl;

    std::unique_ptr<Options> options;
    try {
        options = parseOptions(argc, argv);
        if (!options) {
            return 1;
        }
    } catch (const std::exception& err) {
        std::cout << "[ERROR] " << err.what() << std::endl;
        return 1;
    }

    solvePuzzles(*options);

    return 0;
}

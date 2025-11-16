#pragma once

#include <iosfwd>
#include <vector>

#include "engine/board.h"
#include "utils.h"

SUDOKU_NAMESPACE::serialization {
    struct Puzzle {
        utils::ArrayVector<BoardCage> cages;
        // 81 bytes, row-major 1..9
        BoardState<BoardCell> solution;
    };

    class PuzzleLoader {
    private:
        static constexpr size_t SOLUTION_SIZE = BOARD_SIZE * BOARD_SIZE;
        // 4 magic + 1 version + 3 pad + 4 count
        static constexpr size_t HEADER_SIZE = 12;
        // 'KSF1' little endian
        static constexpr uint32_t MAGIC = 0x3146534B;
        static constexpr uint8_t VERSION = 1;

        std::istream& file;
        utils::ArrayVector<size_t> index_offsets;

    public:
        PuzzleLoader(std::istream& file);
        PuzzleLoader(const PuzzleLoader&) = delete;
        ~PuzzleLoader() = default;

        std::uint32_t puzzle_count() const {
            return static_cast<std::uint32_t>(this->index_offsets.size());
        }

        std::unique_ptr<Puzzle> load_puzzle(size_t index);

    private:
        void read_header();
        void read_index();
    };
}

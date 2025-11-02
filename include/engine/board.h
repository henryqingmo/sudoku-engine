#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>

#include "../base.h"
#include "../utils.h"

SUDOKU_NAMESPACE {
    constexpr int BOARD_SIZE = 9;
    constexpr int BOX_SIZE = 3;

    using BoardCell = std::uint8_t;
    using BoardOffset = std::uint16_t;

    constexpr BoardCell CELL_EMPTY = 0;
    constexpr BoardCell CELL_MIN = 1;
    constexpr BoardCell CELL_MAX = 9;

    struct BoardPosition {
        BoardOffset row;
        BoardOffset col;

        constexpr BoardPosition() : row(0), col(0) {}

        constexpr BoardPosition(BoardOffset row, BoardOffset col)
            : row(row), col(col) {}

        // Convert to index in a flat representation
        std::size_t toOffset() const {
            if (this->row >= BOARD_SIZE || this->col >= BOARD_SIZE)
                throw std::out_of_range("Invalid board position");
            return static_cast<std::size_t>(BOARD_SIZE) * this->row + this->col;
        }

        constexpr bool operator==(const BoardPosition& other) const {
            return this->row == other.row && this->col == other.col;
        }

        struct Hasher {
            std::size_t operator()(const BoardPosition& pos) const {
                return pos.toOffset();
            }
        };
    };

    struct BoardCage {
        std::vector<BoardPosition> cells;
        unsigned sum;

        BoardCage(unsigned sum, std::vector<BoardPosition> cells)
            : cells(std::move(cells)), sum(sum) {}
    };

    class BoardCellDomain {
    private:
        std::bitset<BOARD_SIZE> exists;

    public:
        constexpr BoardCellDomain() = default;

        BoardCellDomain(std::initializer_list<BoardCell> values) {
            for (const BoardCell value : values) {
                this->add(value);
            }
        }

        void add(BoardCell value) {
            this->check(value);
            this->exists.set(value - 1, true);
        }

        void remove(BoardCell value) {
            this->check(value);
            this->exists.set(value - 1, false);
        }

        void flip() {
            this->exists.flip();
        }

        [[nodiscard]]
        bool has(BoardCell value) const {
            this->check(value);
            return this->exists.test(value - 1);
        }

        [[nodiscard]]
        bool empty() const {
            return this->exists.none();
        }

        [[nodiscard]]
        BoardCellDomain operator-() const {
            BoardCellDomain result = *this;
            result.flip();
            return result;
        }

    private:
        void check(BoardCell value) const {
            if (value < CELL_MIN || value > CELL_MAX) {
                throw std::runtime_error("Invalid board cell value");
            }
        }
    };

    template <class Data>
    class BoardState {
    private:
        std::vector<Data> cells;
        std::size_t size;

    public:
        BoardState(std::size_t size, const Data& fill)
            : cells(size * size, fill), size(size) {}

        BoardState(std::size_t size, std::vector<Data> data)
            : size(size), cells(std::move(data)) {
            if (this->cells.size() != size * size) {
                throw std::runtime_error("Raw data vector size mismatch");
            }
        }

        bool contains(const Data& cell_data) const {
            for (const Data& cell : this->cells) {
                if (cell == cell_data)
                    return true;
            }
            return false;
        }

        void reset(const Data& cell_data) {
            for (Data& cell : this->cells) {
                cell = cell_data;
            }
        }

        std::size_t getSize() const {
            return this->size;
        }

        Data& operator[](const BoardPosition& pos) {
            return this->cells[pos.toOffset()];
        }

        const Data& operator[](const BoardPosition& pos) const {
            return this->cells[pos.toOffset()];
        }
    };

    class Board {
    public:
        using LineOrBox = std::array<BoardPosition, BOARD_SIZE>;

    private:
        BoardState<BoardCell> cell_values;
        BoardState<const BoardCage*> cell_cages;
        std::span<const BoardCage> cages;

    public:
        Board()
            : cell_values(BOARD_SIZE, CELL_EMPTY),
              cell_cages(BOARD_SIZE, nullptr) {}

        BoardState<BoardCell>& getValues() {
            return this->cell_values;
        }

        void setValues(BoardState<BoardCell> state) {
            this->cell_values = std::move(state);
        }

        void setCages(const std::span<const BoardCage>& cages);

        bool isInvalid() const;
        bool isInvalid(const BoardPosition& pos) const;

        bool isIncomplete() const {
            return this->cell_values.contains(0);
        }

        const LineOrBox& getRow(BoardOffset index) const;
        const LineOrBox& getCol(BoardOffset index) const;
        const LineOrBox& getBox(BoardOffset index) const;

        constexpr BoardOffset getCellBox(const BoardPosition& pos) const {
            return (pos.row / BOX_SIZE) * BOX_SIZE + (pos.col / BOX_SIZE);
        }

        const BoardCage* getCellCage(const BoardPosition& pos) const {
            return this->cell_cages[pos];
        }

        void print(std::ostream& output) const;

    private:
        bool hasInvalidLines() const;
        bool hasInvalidBoxes() const;
        bool hasInvalidCages() const;

        bool isInvalidLineOrBox(const LineOrBox& cells) const;

        bool isInvalidCage(const BoardCage& cage) const;

        bool isInvalidRow(BoardOffset row) const {
            return this->isInvalidLineOrBox(this->getRow(row));
        }

        bool isInvalidCol(BoardOffset col) const {
            return this->isInvalidLineOrBox(this->getCol(col));
        }
    };
}

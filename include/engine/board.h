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

        constexpr bool operator==(const BoardPosition& other) const {
            return this->row == other.row && this->col == other.col;
        }
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
            return this->cells[this->getPositionOffset(pos)];
        }

        const Data& operator[](const BoardPosition& pos) const {
            return this->cells[this->getPositionOffset(pos)];
        }

    private:
        std::size_t getPositionOffset(const BoardPosition& pos) const {
            if (pos.row >= BOARD_SIZE || pos.col >= BOARD_SIZE)
                throw std::out_of_range("Invalid board position");
            return pos.row * this->size + pos.col;
        }
    };

    class Board {
    public:
        using LineOrBox =
            std::unique_ptr<std::array<BoardPosition, BOARD_SIZE>>;

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

        LineOrBox getLine(BoardOffset index, const BoardPosition& delta) const;
        LineOrBox getBox(BoardOffset index) const;

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

        bool isInvalidLine(BoardOffset index, const BoardPosition& delta) const;
        bool isInvalidBox(BoardOffset index) const;
        bool isInvalidCage(const BoardCage& cage) const;

        bool isInvalidRow(BoardOffset row) const {
            return this->isInvalidLine(row, {0, 1});
        }

        bool isInvalidCol(BoardOffset col) const {
            return this->isInvalidLine(col, {1, 0});
        }
    };
}

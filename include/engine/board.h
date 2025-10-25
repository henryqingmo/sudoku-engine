#pragma once

#include <cstdint>
#include <iosfwd>
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

        constexpr BoardPosition(BoardOffset row, BoardOffset col)
            : row(row), col(col) {}
    };

    struct BoardCage {
        std::vector<BoardPosition> cells;
        unsigned sum;

        BoardCage(unsigned sum, std::vector<BoardPosition> cells)
            : cells(std::move(cells)), sum(sum) {}
    };

    template <class Data>
    class BoardState {
    private:
        std::vector<Data> cells;
        std::size_t size;

    public:
        BoardState(std::size_t size) : cells(size * size, 0), size(size) {}
        BoardState(std::size_t size, std::vector<Data> data)
            : size(size), cells(std::move(data)) {
            if (data.size() != size * size) {
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
    private:
        BoardState<BoardCell> cell_values;
        BoardState<const BoardCage*> cell_cages;
        std::span<const BoardCage> cages;

    public:
        Board() : cell_values(BOARD_SIZE), cell_cages(BOARD_SIZE) {}

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

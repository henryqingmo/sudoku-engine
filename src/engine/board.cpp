#include <bitset>
#include <iostream>

#include "engine/board.h"

using sudoku_engine::Board;

static inline Board::LineOrBox makeLineOrBox() {
    using namespace sudoku_engine;
    return std::make_unique<std::array<BoardPosition, BOARD_SIZE>>();
}

Board::LineOrBox Board::getLine(
    BoardOffset index,
    const BoardPosition& delta
) const {
    if (delta.row > 1 || delta.col > 1 || (delta.row == 0 && delta.col == 0)) {
        throw std::runtime_error(
            "Delta magnitude cannot be strictly between 0 and sqrt(2)"
        );
    }

    BoardPosition pos = BoardPosition{
        static_cast<BoardOffset>((1 - delta.row) * index),
        static_cast<BoardOffset>((1 - delta.col) * index)
    };

    LineOrBox line = makeLineOrBox();
    for (std::size_t i = 0; pos.col < BOARD_SIZE && pos.row < BOARD_SIZE; i++) {
        line->at(i) = pos;
        pos.row += delta.row;
        pos.col += delta.col;
    }

    return line;
}

Board::LineOrBox Board::getBox(BoardOffset index) const {
    // 0 1 2
    // 3 4 5
    // 6 7 8
    BoardOffset start_row = (index / 3) * 3;
    BoardOffset start_col = (index % 3) * 3;

    LineOrBox box = makeLineOrBox();

    std::size_t i = 0;
    for (BoardOffset row = start_row; row < start_row + 3; row++) {
        for (BoardOffset col = start_col; col < start_col + 3; col++) {
            box->at(i++) = {row, col};
        }
    }

    return box;
}

bool Board::isInvalid() const {
    return this->hasInvalidLines() || this->hasInvalidBoxes() ||
           this->hasInvalidCages();
}

bool Board::isInvalidLine(BoardOffset index, const BoardPosition& delta) const {
    // Represents whether each of 1-9 has already appeared
    BoardCellDomain existing;

    const auto line = this->getLine(index, delta);
    for (const auto& cell_pos : *line) {
        const BoardCell& value = this->cell_values[cell_pos];
        if (value != CELL_EMPTY) {
            if (existing.has(value))
                return true;
            existing.add(value);
        }
    }

    return false;
}

bool Board::isInvalidBox(BoardOffset index) const {
    BoardCellDomain existing;

    const auto box = this->getBox(index);
    for (const auto& cell_pos : *box) {
        const BoardCell& value = this->cell_values[cell_pos];
        if (value != CELL_EMPTY) {
            if (existing.has(value))
                return true;
            existing.add(value);
        }
    }

    return false;
}

bool Board::isInvalidCage(const BoardCage& cage) const {
    BoardCellDomain existing;

    // Check if num already exists in the cage (killer sudoku rule: no duplicates in cage)
    for (const auto& pos : cage.cells) {
        const BoardCell& value = this->cell_values[pos];
        if (value != CELL_EMPTY) {
            if (existing.has(value))
                return true;
            existing.add(value);
        }
    }

    // Calculate current sum and count filled cells
    unsigned current_sum = 0;
    BoardOffset filled_count = 0;
    BoardOffset empty_count = 0;

    for (const auto& pos : cage.cells) {
        const BoardCell& value = this->cell_values[pos];
        if (value != CELL_EMPTY) {
            current_sum += value;
            filled_count++;
        } else {
            empty_count++;
        }
    }

    // If sum already exceeds target, invalid
    if (current_sum > cage.sum) {
        return true;
    }

    // If cage is complete, check if sum matches
    if (empty_count == 0) {
        return current_sum != cage.sum;
    }

    // Check if remaining cells can possibly reach the target sum
    // Minimum possible remaining: 1 * emptyCount
    // Maximum possible remaining: 9 * emptyCount (but constrained by sudoku rules)
    const int remaining = static_cast<int>(cage.sum) - current_sum;

    if (remaining < empty_count || remaining > 9 * empty_count) {
        return true;
    }

    return false;
}

bool Board::hasInvalidLines() const {
    for (BoardOffset index = 0; index < BOARD_SIZE; index++) {
        if (this->isInvalidRow(index) || this->isInvalidCol(index)) {
            return true;
        }
    }
    return false;
}

bool Board::hasInvalidBoxes() const {
    for (BoardOffset index = 0; index < BOARD_SIZE; index++) {
        if (this->isInvalidBox(index)) {
            return true;
        }
    }
    return false;
}

bool Board::hasInvalidCages() const {
    for (const auto& cage : this->cages) {
        if (this->isInvalidCage(cage)) {
            return true;
        }
    }
    return false;
}

bool Board::isInvalid(const BoardPosition& pos) const {
    if (this->isInvalidRow(pos.row) || this->isInvalidCol(pos.col)) {
        return true;
    }

    const BoardOffset box = this->getCellBox(pos);
    if (this->isInvalidBox(box)) {
        return true;
    }

    const BoardCage* const cage = this->cell_cages[pos];
    if (cage != nullptr && this->isInvalidCage(*cage)) {
        return true;
    }

    return false;
}

void Board::setCages(const std::span<const BoardCage>& new_cages) {
    this->cages = new_cages;
    this->cell_cages.reset(nullptr);
    for (const auto& cage : this->cages) {
        for (const auto& cell_pos : cage.cells) {
            if (this->cell_cages[cell_pos] != nullptr) {
                throw std::runtime_error("Overlapping cages detected");
            }
            this->cell_cages[cell_pos] = &cage;
        }
    }
}

void Board::print(std::ostream& output) const {
    output << "+-------+-------+-------+" << std::endl;
    for (BoardOffset row = 0; row < 9; row++) {
        output << "| ";
        for (BoardOffset col = 0; col < 9; col++) {
            const auto& value = this->cell_values[{row, col}];

            if (value == CELL_EMPTY) {
                output << " ";
            } else {
                output << static_cast<unsigned>(value);
            }

            if (col % 3 == 2) {
                output << " | ";
            } else {
                output << " ";
            }
        }
        output << std::endl;

        if (row % 3 == 2 && row != 8) {
            output << "|-------|-------|-------|" << std::endl;
        }
    }
    output << "+-------+-------+-------+" << std::endl;
}

#include <bitset>
#include <iostream>

#include "engine/board.h"

using sudoku_engine::Board;

bool Board::isInvalid() const {
    return this->hasInvalidLines() || this->hasInvalidBoxes() ||
           this->hasInvalidCages();
}

bool Board::isInvalidLine(BoardOffset index, const BoardPosition& delta) const {
    if (delta.row > 1 || delta.col > 1 || (delta.row == 0 && delta.col == 0)) {
        throw std::runtime_error(
            "Delta magnitude cannot be strictly between 0 and sqrt(2)"
        );
    }

    // Represents whether each of 1-9 has already appeared
    std::bitset<BOARD_SIZE> number_exists;

    BoardPosition pos = BoardPosition{
        static_cast<BoardOffset>((1 - delta.row) * index),
        static_cast<BoardOffset>((1 - delta.col) * index)
    };

    while (pos.col < BOARD_SIZE && pos.row < BOARD_SIZE) {
        const BoardCell& value = this->cell_values[pos];
        if (value != CELL_EMPTY) {
            if (number_exists.test(value - 1))
                return true;
            number_exists.set(value - 1, true);
        }
        pos.row += delta.row;
        pos.col += delta.col;
    }

    return false;
}

bool Board::isInvalidBox(BoardOffset index) const {
    // 0 1 2
    // 3 4 5
    // 6 7 8
    BoardOffset startRow = (index / 3) * 3;
    BoardOffset startCol = (index % 3) * 3;

    std::bitset<BOARD_SIZE> number_exists;

    for (BoardOffset row = startRow; row < startRow + 3; row++) {
        for (BoardOffset col = startCol; col < startCol + 3; col++) {
            const BoardCell& value = this->cell_values[{row, col}];
            if (value != CELL_EMPTY) {
                if (number_exists.test(value - 1))
                    return true;
                number_exists.set(value - 1, true);
            }
        }
    }

    return false;
}

bool Board::isInvalidCage(const BoardCage& cage) const {
    std::bitset<BOARD_SIZE> number_exists;

    // Check if num already exists in the cage (killer sudoku rule: no duplicates in cage)
    for (const auto& pos : cage.cells) {
        const BoardCell& value = this->cell_values[pos];
        if (value != CELL_EMPTY) {
            if (number_exists.test(value - 1))
                return true;
            number_exists.set(value - 1, true);
        }
    }

    // Calculate current sum and count filled cells
    unsigned currentSum = 0;
    BoardOffset filledCount = 1;  // Including the value we're trying to place
    BoardOffset emptyCount = 0;

    for (const auto& pos : cage.cells) {
        const BoardCell& value = this->cell_values[pos];
        if (value != CELL_EMPTY) {
            currentSum += value;
            filledCount++;
        } else {
            emptyCount++;
        }
    }

    // If sum already exceeds target, invalid
    if (currentSum > cage.sum) {
        return true;
    }

    // If cage is complete, check if sum matches
    if (emptyCount == 0) {
        return currentSum != cage.sum;
    }

    // Check if remaining cells can possibly reach the target sum
    // Minimum possible remaining: 1 * emptyCount
    // Maximum possible remaining: 9 * emptyCount (but constrained by sudoku rules)
    const int remaining = static_cast<int>(cage.sum) - currentSum;

    if (remaining < emptyCount || remaining > 9 * emptyCount) {
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

    const BoardOffset box = (pos.row / 3) * 3 + (pos.col / 3);
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

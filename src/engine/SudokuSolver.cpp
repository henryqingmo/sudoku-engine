#include "engine/SudokuSolver.h"
#include <iostream>
#include <numeric>

SudokuSolver::SudokuSolver() : board_(9, std::vector<int>(9, 0)) {
    // Initialize empty 9x9 board
}

SudokuSolver::~SudokuSolver() {
    // Destructor
}

void SudokuSolver::init() {
    std::cout << "Killer Sudoku solver initialized" << std::endl;
}

void SudokuSolver::setCages(const std::vector<Cage>& cages) {
    cages_ = cages;
}

void SudokuSolver::setBoard(const std::vector<std::vector<int>>& board) {
    board_ = board;
}

bool SudokuSolver::solve() {
    return solveBacktrack(0, 0);
}

bool SudokuSolver::solveBacktrack(int row, int col) {
    // Move to next row if we've reached the end of current row
    if (col == 9) {
        row++;
        col = 0;
    }
    
    // If we've filled all rows, we're done
    if (row == 9) {
        return true;
    }
    
    // Skip cells that are already filled
    if (board_[row][col] != 0) {
        return solveBacktrack(row, col + 1);
    }
    
    // Try placing numbers 1-9
    for (int num = 1; num <= 9; num++) {
        if (isValidPlacement(row, col, num)) {
            board_[row][col] = num;
            
            // Recursively try to fill the rest
            if (solveBacktrack(row, col + 1)) {
                return true;
            }
            
            // Backtrack if this placement doesn't lead to a solution
            board_[row][col] = 0;
        }
    }
    
    return false;
}

bool SudokuSolver::isValidPlacement(int row, int col, int num) const {
    return isValidRow(row, num) && 
           isValidCol(col, num) && 
           isValidBox(row, col, num) && 
           isValidCage(row, col, num);
}

bool SudokuSolver::isValidRow(int row, int num) const {
    for (int col = 0; col < 9; col++) {
        if (board_[row][col] == num) {
            return false;
        }
    }
    return true;
}

bool SudokuSolver::isValidCol(int col, int num) const {
    for (int row = 0; row < 9; row++) {
        if (board_[row][col] == num) {
            return false;
        }
    }
    return true;
}

bool SudokuSolver::isValidBox(int row, int col, int num) const {
    int boxRow = (row / 3) * 3;
    int boxCol = (col / 3) * 3;
    
    for (int r = boxRow; r < boxRow + 3; r++) {
        for (int c = boxCol; c < boxCol + 3; c++) {
            if (board_[r][c] == num) {
                return false;
            }
        }
    }
    return true;
}

bool SudokuSolver::isValidCage(int row, int col, int num) const {
    int cageIdx = findCage(row, col);
    if (cageIdx == -1) {
        return true; // No cage constraint for this cell
    }
    
    const Cage& cage = cages_[cageIdx];
    
    // Check if num already exists in the cage (killer sudoku rule: no duplicates in cage)
    for (const auto& [r, c] : cage.cells) {
        if (board_[r][c] == num) {
            return false;
        }
    }
    
    // Calculate current sum and count filled cells
    int currentSum = num;
    int filledCount = 1; // Including the number we're trying to place
    int emptyCount = 0;
    
    for (const auto& [r, c] : cage.cells) {
        if (r == row && c == col) {
            continue; // Skip the cell we're trying to fill
        }
        
        if (board_[r][c] != 0) {
            currentSum += board_[r][c];
            filledCount++;
        } else {
            emptyCount++;
        }
    }
    
    // If sum already exceeds target, invalid
    if (currentSum > cage.sum) {
        return false;
    }
    
    // If cage is complete, check if sum matches
    if (emptyCount == 0) {
        return currentSum == cage.sum;
    }
    
    // Check if remaining cells can possibly reach the target sum
    // Minimum possible remaining: 1 * emptyCount
    // Maximum possible remaining: 9 * emptyCount (but constrained by sudoku rules)
    int remaining = cage.sum - currentSum;
    
    if (remaining < emptyCount || remaining > 9 * emptyCount) {
        return false;
    }
    
    return true;
}

int SudokuSolver::findCage(int row, int col) const {
    for (size_t i = 0; i < cages_.size(); i++) {
        for (const auto& [r, c] : cages_[i].cells) {
            if (r == row && c == col) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

bool SudokuSolver::isCageComplete(const Cage& cage) const {
    for (const auto& [r, c] : cage.cells) {
        if (board_[r][c] == 0) {
            return false;
        }
    }
    return true;
}

bool SudokuSolver::isCageValid(const Cage& cage) const {
    if (!isCageComplete(cage)) {
        return false;
    }
    
    int sum = 0;
    for (const auto& [r, c] : cage.cells) {
        sum += board_[r][c];
    }
    
    return sum == cage.sum;
}

void SudokuSolver::printBoard() const {
    std::cout << "\n╔═══════╤═══════╤═══════╗\n";
    for (int row = 0; row < 9; row++) {
        std::cout << "║ ";
        for (int col = 0; col < 9; col++) {
            if (board_[row][col] == 0) {
                std::cout << "·";
            } else {
                std::cout << board_[row][col];
            }
            
            if (col % 3 == 2) {
                std::cout << " ║ ";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "\n";
        
        if (row % 3 == 2 && row != 8) {
            std::cout << "╟───────┼───────┼───────╢\n";
        }
    }
    std::cout << "╚═══════╧═══════╧═══════╝\n";
}

std::vector<std::vector<int>> SudokuSolver::getBoard() const {
    return board_;
}

#ifndef SUDOKU_ENGINE_H
#define SUDOKU_ENGINE_H

#include <vector>
#include <utility>

// Represents a cage in killer sudoku
struct Cage {
    int sum;                              // Target sum for this cage
    std::vector<std::pair<int, int>> cells; // (row, col) coordinates
};

class SudokuSolver {
public:
    SudokuSolver();
    ~SudokuSolver();
    
    void init();
    
    // Set up a killer sudoku puzzle
    void setCages(const std::vector<Cage>& cages);
    void setBoard(const std::vector<std::vector<int>>& board);
    
    // Solve the killer sudoku puzzle
    bool solve();
    
    // Print the board
    void printBoard() const;
    
    // Get the solved board
    std::vector<std::vector<int>> getBoard() const;
    
private:
    std::vector<std::vector<int>> board_;  // 9x9 sudoku grid (0 = empty)
    std::vector<Cage> cages_;              // Cage constraints
    
    // Backtracking solver
    bool solveBacktrack(int row, int col);
    
    // Validation methods
    bool isValidPlacement(int row, int col, int num) const;
    bool isValidRow(int row, int num) const;
    bool isValidCol(int col, int num) const;
    bool isValidBox(int row, int col, int num) const;
    bool isValidCage(int row, int col, int num) const;
    
    // Helper to find which cage a cell belongs to
    int findCage(int row, int col) const;
    
    // Check if a cage is complete and valid
    bool isCageComplete(const Cage& cage) const;
    bool isCageValid(const Cage& cage) const;
};

#endif // SUDOKU_ENGINE_H

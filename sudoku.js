class SudokuEngine {
    constructor() {
        this.grid = Array(9).fill().map(() => Array(9).fill(0));
        this.solution = Array(9).fill().map(() => Array(9).fill(0));
        this.givenCells = new Set();
        this.gameMode = 'normal'; // 'normal' or 'killer'
        this.cages = []; // For Killer Sudoku: array of {cells: [{row, col}], sum: number}
    }

    // Generate a complete valid Sudoku solution
    generateSolution() {
        this.solution = Array(9).fill().map(() => Array(9).fill(0));
        
        // Fill diagonal 3x3 boxes first (they are independent)
        for (let i = 0; i < 9; i += 3) {
            this.fillBox(i, i);
        }
        
        // Fill remaining cells using backtracking
        this.solveGrid(this.solution);
    }

    // Fill a 3x3 box with random numbers 1-9
    fillBox(row, col) {
        const numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9];
        this.shuffleArray(numbers);
        
        let index = 0;
        for (let i = 0; i < 3; i++) {
            for (let j = 0; j < 3; j++) {
                this.solution[row + i][col + j] = numbers[index++];
            }
        }
    }

    // Shuffle array using Fisher-Yates algorithm
    shuffleArray(array) {
        for (let i = array.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [array[i], array[j]] = [array[j], array[i]];
        }
    }

    // Solve grid using backtracking
    solveGrid(grid) {
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (grid[row][col] === 0) {
                    for (let num = 1; num <= 9; num++) {
                        if (this.isValidMove(grid, row, col, num)) {
                            grid[row][col] = num;
                            
                            if (this.solveGrid(grid)) {
                                return true;
                            }
                            
                            grid[row][col] = 0;
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }

    // Check if a move is valid
    isValidMove(grid, row, col, num) {
        // Check row
        for (let x = 0; x < 9; x++) {
            if (grid[row][x] === num) return false;
        }
        
        // Check column
        for (let x = 0; x < 9; x++) {
            if (grid[x][col] === num) return false;
        }
        
        // Check 3x3 box
        const startRow = Math.floor(row / 3) * 3;
        const startCol = Math.floor(col / 3) * 3;
        
        for (let i = 0; i < 3; i++) {
            for (let j = 0; j < 3; j++) {
                if (grid[startRow + i][startCol + j] === num) return false;
            }
        }
        
        return true;
    }

    // Generate puzzle by removing cells from solution
    generatePuzzle(difficulty = 'medium') {
        this.generateSolution();
        
        // Copy solution to grid
        this.grid = this.solution.map(row => [...row]);
        
        // Determine number of cells to remove based on difficulty
        let cellsToRemove;
        switch (difficulty) {
            case 'easy':
                cellsToRemove = 40;
                break;
            case 'medium':
                cellsToRemove = 50;
                break;
            case 'hard':
                cellsToRemove = 60;
                break;
            default:
                cellsToRemove = 50;
        }
        
        // Track given cells
        this.givenCells.clear();
        
        // Remove cells randomly
        let removed = 0;
        while (removed < cellsToRemove) {
            const row = Math.floor(Math.random() * 9);
            const col = Math.floor(Math.random() * 9);
            
            if (this.grid[row][col] !== 0) {
                this.grid[row][col] = 0;
                removed++;
            }
        }
        
        // Mark remaining cells as given
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (this.grid[row][col] !== 0) {
                    this.givenCells.add(`${row}-${col}`);
                }
            }
        }
    }

    // Check if current grid is valid (no conflicts)
    isValidGrid() {
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                const num = this.grid[row][col];
                if (num !== 0) {
                    // Temporarily remove the number to check validity
                    this.grid[row][col] = 0;
                    const isValid = this.isValidMove(this.grid, row, col, num);
                    this.grid[row][col] = num;
                    
                    if (!isValid) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Check if puzzle is complete and correct
    isComplete() {
        // Check if all cells are filled
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (this.grid[row][col] === 0) {
                    return false;
                }
            }
        }
        
        // Check if grid matches solution
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (this.grid[row][col] !== this.solution[row][col]) {
                    return false;
                }
            }
        }
        
        return true;
    }

    // Get conflicts for a specific cell
    getConflicts(row, col) {
        const num = this.grid[row][col];
        if (num === 0) return [];
        
        const conflicts = [];
        
        // Check row
        for (let x = 0; x < 9; x++) {
            if (x !== col && this.grid[row][x] === num) {
                conflicts.push({row, col: x});
            }
        }
        
        // Check column
        for (let x = 0; x < 9; x++) {
            if (x !== row && this.grid[x][col] === num) {
                conflicts.push({row: x, col});
            }
        }
        
        // Check 3x3 box
        const startRow = Math.floor(row / 3) * 3;
        const startCol = Math.floor(col / 3) * 3;
        
        for (let i = 0; i < 3; i++) {
            for (let j = 0; j < 3; j++) {
                const checkRow = startRow + i;
                const checkCol = startCol + j;
                if ((checkRow !== row || checkCol !== col) && 
                    this.grid[checkRow][checkCol] === num) {
                    conflicts.push({row: checkRow, col: checkCol});
                }
            }
        }
        
        return conflicts;
    }

    // Solve the current puzzle using backtracking
    solve() {
        const workingGrid = this.grid.map(row => [...row]);
        const solved = this.solveGrid(workingGrid);
        
        if (solved) {
            this.grid = workingGrid;
            return true;
        }
        return false;
    }

    // Get hint for a random empty cell
    getHint() {
        const emptyCells = [];
        
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (this.grid[row][col] === 0) {
                    emptyCells.push({row, col});
                }
            }
        }
        
        if (emptyCells.length === 0) {
            return null; // No empty cells
        }
        
        const randomCell = emptyCells[Math.floor(Math.random() * emptyCells.length)];
        const correctValue = this.solution[randomCell.row][randomCell.col];
        
        return {
            row: randomCell.row,
            col: randomCell.col,
            value: correctValue
        };
    }

    // Check if a cell is given (part of original puzzle)
    isGivenCell(row, col) {
        return this.givenCells.has(`${row}-${col}`);
    }

    // Set value in grid
    setValue(row, col, value) {
        if (!this.isGivenCell(row, col)) {
            this.grid[row][col] = value;
            return true;
        }
        return false;
    }

    // Get value from grid
    getValue(row, col) {
        return this.grid[row][col];
    }

    // Clear user input (keep given cells)
    clearUserInput() {
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (!this.isGivenCell(row, col)) {
                    this.grid[row][col] = 0;
                }
            }
        }
    }

    // Set game mode
    setGameMode(mode) {
        this.gameMode = mode;
        if (mode === 'killer') {
            this.cages = [];
            // TODO: Implement Killer Sudoku cage generation
            // This will be implemented later
        } else {
            this.cages = [];
        }
    }

    // Get game mode
    getGameMode() {
        return this.gameMode;
    }

    // Get cages for Killer Sudoku
    getCages() {
        return this.cages;
    }

    // Check if Killer Sudoku cage constraints are satisfied
    isValidKillerCage(cage) {
        const cageCells = cage.cells;
        const cageSum = cage.sum;
        
        // Check if all cells in cage are filled
        const filledCells = cageCells.filter(cell => this.grid[cell.row][cell.col] !== 0);
        
        if (filledCells.length !== cageCells.length) {
            return true; // Incomplete cage is valid
        }
        
        // Check if sum matches
        const actualSum = filledCells.reduce((sum, cell) => sum + this.grid[cell.row][cell.col], 0);
        return actualSum === cageSum;
    }

    // Check if all Killer Sudoku cages are valid
    isValidKillerSudoku() {
        if (this.gameMode !== 'killer') {
            return true;
        }
        
        return this.cages.every(cage => this.isValidKillerCage(cage));
    }

    // Get Killer Sudoku cage conflicts
    getKillerConflicts() {
        const conflicts = [];
        
        if (this.gameMode !== 'killer') {
            return conflicts;
        }
        
        this.cages.forEach(cage => {
            if (!this.isValidKillerCage(cage)) {
                conflicts.push(...cage.cells);
            }
        });
        
        return conflicts;
    }

    // Generate Killer Sudoku puzzle (placeholder for future implementation)
    generateKillerPuzzle(difficulty = 'medium') {
        // TODO: Implement Killer Sudoku generation
        // For now, fall back to normal Sudoku
        this.generatePuzzle(difficulty);
        // Note: Status message will be shown by the UI layer
    }
}

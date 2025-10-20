class SudokuGame {
    constructor() {
        this.sudoku = new SudokuEngine();
        this.selectedCell = null;
        this.timer = 0;
        this.timerInterval = null;
        this.gameStarted = false;
        
        this.initializeElements();
        this.bindEvents();
        this.createGrid();
        this.newGame();
    }

    initializeElements() {
        this.gridElement = document.getElementById('sudoku-grid');
        this.newGameBtn = document.getElementById('newGame');
        this.hintBtn = document.getElementById('hint');
        this.solveBtn = document.getElementById('solve');
        this.checkBtn = document.getElementById('check');
        this.difficultySelect = document.getElementById('difficulty');
        this.gameModeSelect = document.getElementById('gameMode');
        this.timerElement = document.getElementById('timer');
        this.statusMessage = document.getElementById('statusMessage');
    }

    bindEvents() {
        this.newGameBtn.addEventListener('click', () => this.newGame());
        this.hintBtn.addEventListener('click', () => this.giveHint());
        this.solveBtn.addEventListener('click', () => this.solvePuzzle());
        this.checkBtn.addEventListener('click', () => this.checkSolution());
        this.difficultySelect.addEventListener('change', () => this.newGame());
        this.gameModeSelect.addEventListener('change', () => this.switchGameMode());
        
        // Keyboard events
        document.addEventListener('keydown', (e) => this.handleKeyPress(e));
        
        // Prevent default behavior for number keys
        document.addEventListener('keypress', (e) => {
            if (e.key >= '1' && e.key <= '9') {
                e.preventDefault();
            }
        });
    }

    createGrid() {
        this.gridElement.innerHTML = '';
        this.cells = [];
        
        for (let row = 0; row < 9; row++) {
            this.cells[row] = [];
            for (let col = 0; col < 9; col++) {
                const cell = document.createElement('div');
                cell.className = 'cell';
                cell.dataset.row = row;
                cell.dataset.col = col;
                
                cell.addEventListener('click', () => this.selectCell(row, col));
                
                this.gridElement.appendChild(cell);
                this.cells[row][col] = cell;
            }
        }
    }

    selectCell(row, col) {
        // Deselect previous cell
        if (this.selectedCell) {
            this.cells[this.selectedCell.row][this.selectedCell.col].classList.remove('selected');
        }
        
        // Don't allow selection of given cells
        if (this.sudoku.isGivenCell(row, col)) {
            return;
        }
        
        // Select new cell
        this.selectedCell = { row, col };
        this.cells[row][col].classList.add('selected');
    }

    handleKeyPress(e) {
        if (!this.selectedCell) return;
        
        const { row, col } = this.selectedCell;
        
        // Number keys 1-9
        if (e.key >= '1' && e.key <= '9') {
            this.setCellValue(row, col, parseInt(e.key));
        }
        // Delete/Backspace
        else if (e.key === 'Delete' || e.key === 'Backspace') {
            this.setCellValue(row, col, 0);
        }
        // Arrow keys for navigation
        else if (e.key === 'ArrowUp' && row > 0) {
            this.selectCell(row - 1, col);
        }
        else if (e.key === 'ArrowDown' && row < 8) {
            this.selectCell(row + 1, col);
        }
        else if (e.key === 'ArrowLeft' && col > 0) {
            this.selectCell(row, col - 1);
        }
        else if (e.key === 'ArrowRight' && col < 8) {
            this.selectCell(row, col + 1);
        }
    }

    setCellValue(row, col, value) {
        if (this.sudoku.setValue(row, col, value)) {
            this.updateCellDisplay(row, col);
            this.validateAndHighlightErrors();
            
            if (!this.gameStarted) {
                this.startTimer();
            }
            
            // Check if puzzle is complete
            if (this.sudoku.isComplete()) {
                this.gameComplete();
            }
        }
    }

    updateCellDisplay(row, col) {
        const cell = this.cells[row][col];
        const value = this.sudoku.getValue(row, col);
        
        // Clear previous classes
        cell.classList.remove('user-input', 'error', 'hint');
        
        if (value === 0) {
            cell.textContent = '';
        } else {
            cell.textContent = value;
            
            if (this.sudoku.isGivenCell(row, col)) {
                cell.classList.add('given');
            } else {
                cell.classList.add('user-input');
            }
        }
    }

    validateAndHighlightErrors() {
        // Clear all error highlighting
        this.cells.forEach(row => {
            row.forEach(cell => {
                cell.classList.remove('error');
            });
        });
        
        // Check for conflicts and highlight errors
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                const conflicts = this.sudoku.getConflicts(row, col);
                if (conflicts.length > 0) {
                    this.cells[row][col].classList.add('error');
                    conflicts.forEach(conflict => {
                        this.cells[conflict.row][conflict.col].classList.add('error');
                    });
                }
            }
        }
        
        // Check Killer Sudoku cage conflicts if in killer mode
        if (this.sudoku.getGameMode() === 'killer') {
            const killerConflicts = this.sudoku.getKillerConflicts();
            killerConflicts.forEach(conflict => {
                this.cells[conflict.row][conflict.col].classList.add('error');
            });
        }
    }

    newGame() {
        const difficulty = this.difficultySelect.value;
        const gameMode = this.gameModeSelect.value;
        
        this.sudoku.setGameMode(gameMode);
        
        if (gameMode === 'killer') {
            this.sudoku.generateKillerPuzzle(difficulty);
            this.showStatus('Killer Sudoku generation not yet implemented. Using normal Sudoku.', 'info');
        } else {
            this.sudoku.generatePuzzle(difficulty);
        }
        
        this.updateDisplay();
        this.resetTimer();
        this.gameStarted = false;
        this.selectedCell = null;
        
        const modeText = gameMode === 'killer' ? 'Killer Sudoku' : 'Normal Sudoku';
        this.showStatus(`New ${modeText} game started! Good luck!`, 'info');
    }

    switchGameMode() {
        const gameMode = this.gameModeSelect.value;
        this.sudoku.setGameMode(gameMode);
        
        // Update grid styling for killer mode
        if (gameMode === 'killer') {
            this.gridElement.classList.add('killer-mode');
            this.showStatus('Switched to Killer Sudoku mode. Cage generation not yet implemented.', 'info');
        } else {
            this.gridElement.classList.remove('killer-mode');
            this.showStatus('Switched to Normal Sudoku mode.', 'info');
        }
        
        // Generate new game with current settings
        this.newGame();
    }

    updateDisplay() {
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                this.updateCellDisplay(row, col);
            }
        }
        this.validateAndHighlightErrors();
    }

    giveHint() {
        const hint = this.sudoku.getHint();
        
        if (!hint) {
            this.showStatus('No empty cells to hint!', 'info');
            return;
        }
        
        const { row, col, value } = hint;
        this.sudoku.setValue(row, col, value);
        this.updateCellDisplay(row, col);
        this.cells[row][col].classList.add('hint');
        
        // Remove hint animation class after animation completes
        setTimeout(() => {
            this.cells[row][col].classList.remove('hint');
        }, 1000);
        
        this.showStatus(`Hint: ${value} at row ${row + 1}, column ${col + 1}`, 'info');
        
        // Check if puzzle is complete
        if (this.sudoku.isComplete()) {
            this.gameComplete();
        }
    }

    solvePuzzle() {
        if (this.sudoku.solve()) {
            this.updateDisplay();
            this.showStatus('Puzzle solved using backtracking algorithm!', 'success');
        } else {
            this.showStatus('Unable to solve this puzzle!', 'error');
        }
    }

    checkSolution() {
        if (!this.sudoku.isValidGrid()) {
            this.showStatus('There are conflicts in your solution. Check the red highlighted cells.', 'error');
            return;
        }
        
        // Check Killer Sudoku constraints if in killer mode
        if (this.sudoku.getGameMode() === 'killer' && !this.sudoku.isValidKillerSudoku()) {
            this.showStatus('Killer Sudoku cage constraints not satisfied. Check the red highlighted cells.', 'error');
            return;
        }
        
        if (this.sudoku.isComplete()) {
            this.showStatus('Congratulations! You solved the puzzle correctly!', 'success');
            this.stopTimer();
        } else {
            const emptyCells = this.countEmptyCells();
            this.showStatus(`Puzzle not complete. ${emptyCells} cells remaining.`, 'info');
        }
    }

    countEmptyCells() {
        let count = 0;
        for (let row = 0; row < 9; row++) {
            for (let col = 0; col < 9; col++) {
                if (this.sudoku.getValue(row, col) === 0) {
                    count++;
                }
            }
        }
        return count;
    }

    gameComplete() {
        this.stopTimer();
        this.showStatus('🎉 Congratulations! You solved the puzzle! 🎉', 'success');
        this.gameStarted = false;
    }

    startTimer() {
        if (!this.gameStarted) {
            this.gameStarted = true;
            this.timerInterval = setInterval(() => {
                this.timer++;
                this.updateTimerDisplay();
            }, 1000);
        }
    }

    stopTimer() {
        if (this.timerInterval) {
            clearInterval(this.timerInterval);
            this.timerInterval = null;
        }
    }

    resetTimer() {
        this.stopTimer();
        this.timer = 0;
        this.updateTimerDisplay();
    }

    updateTimerDisplay() {
        const minutes = Math.floor(this.timer / 60);
        const seconds = this.timer % 60;
        this.timerElement.textContent = 
            `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
    }

    showStatus(message, type = 'info') {
        this.statusMessage.textContent = message;
        this.statusMessage.className = `status-message ${type}`;
        
        // Auto-hide after 3 seconds for info messages
        if (type === 'info') {
            setTimeout(() => {
                this.statusMessage.classList.add('hidden');
            }, 3000);
        }
    }
}

// Initialize the game when the page loads
document.addEventListener('DOMContentLoaded', () => {
    new SudokuGame();
});

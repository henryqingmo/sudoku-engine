import { reactive } from "vue";

export interface CellCoords {
  row: number;
  col: number;
}

export interface Cage {
  id: string;
  cells: CellCoords[];
  sum: number;
  color?: string; // For visualization
}

export interface CellState {
  value: number | null;
  domain: Set<number>;
  isFixed: boolean; // If it was part of the initial puzzle
  cageId?: string;
}

export type BoardState = CellState[][];

export class SudokuEngine {
  private board: BoardState;
  private cages: Map<string, Cage>;

  constructor() {
    this.board = reactive(this.createEmptyBoard());
    this.cages = reactive(new Map());
  }

  private createEmptyBoard(): BoardState {
    return Array.from({ length: 9 }, () =>
      Array.from({ length: 9 }, () => ({
        value: null,
        domain: new Set([1, 2, 3, 4, 5, 6, 7, 8, 9]),
        isFixed: false,
      })),
    );
  }

  // Initialize the board with values (0 or null for empty)
  public loadPuzzle(values: (number | null)[][]) {
    for (let r = 0; r < 9; r++) {
      for (let c = 0; c < 9; c++) {
        const row = values[r];
        if (!row) continue;
        const val = row[c];

        const cell = this.board[r]?.[c];
        if (cell) {
          cell.value = (val === 0 || val === undefined) ? null : val;
          cell.isFixed = cell.value !== null;
        }
      }
    }
  }

  public addCage(cage: Cage) {
    this.cages.set(cage.id, cage);
    for (const cell of cage.cells) {
      if (this.isValidCell(cell.row, cell.col)) {
        const boardCell = this.board[cell.row]?.[cell.col];
        if (boardCell) {
          boardCell.cageId = cage.id;
        }
      }
    }
  }

  public getBoard(): BoardState {
    return this.board;
  }

  public getCages(): Cage[] {
    return Array.from(this.cages.values());
  }

  public getCage(id: string): Cage | undefined {
    return this.cages.get(id);
  }

  // Placeholder for solver integration
  public async solve(): Promise<boolean> {
    console.log("Solving...");
    // TODO: Call WASM solver
    return true;
  }

  public async step(): Promise<boolean> {
    console.log("Stepping...");
    // TODO: Call WASM solver step
    return true;
  }

  public reset() {
    // Mutate existing board to preserve references
    for (let r = 0; r < 9; r++) {
      for (let c = 0; c < 9; c++) {
        const cell = this.board[r]?.[c];
        if (cell) {
          cell.value = null;
          cell.domain = new Set([1, 2, 3, 4, 5, 6, 7, 8, 9]);
          cell.isFixed = false;
          cell.cageId = undefined;
        }
      }
    }
    this.cages.clear();
  }

  private isValidCell(row: number, col: number): boolean {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
  }
}

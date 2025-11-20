import createSudokuModule from "../../public/wasm/sudoku-engine";

export interface CellCoords {
  row: number;
  col: number;
}

export interface Cage {
  id: number;
  cells: CellCoords[];
  sum: number;
  color?: string; // For visualization
}

export interface CellState {
  value: number | null;
  domain: Set<number>;
  cage?: Cage;
}

export type BoardState = CellState[][];

// Hard-coded for now...
const EXAMPLE_CAGES_RAW: [number, [number, number][]][] = [
  [6, [[5, 1]]],
  [7, [[8, 4]]],
  [13, [[0, 0], [1, 0]]],
  [14, [[2, 0], [2, 1]]],
  [3, [[0, 1], [1, 1]]],
  [4, [[3, 0], [3, 1]]],
  [14, [[4, 0], [4, 1]]],
  [15, [[5, 0], [6, 0]]],
  [5, [[7, 0], [8, 0]]],
  [8, [[3, 2], [3, 3]]],
  [15, [[4, 2], [4, 3]]],
  [10, [[6, 2], [6, 3]]],
  [4, [[1, 3], [2, 3]]],
  [16, [[2, 4], [2, 5]]],
  [13, [[3, 4], [3, 5]]],
  [7, [[6, 4], [7, 4]]],
  [7, [[6, 5], [7, 5]]],
  [9, [[0, 8], [1, 8]]],
  [10, [[6, 7], [7, 7]]],
  [6, [[6, 8], [7, 8]]],
  [17, [[8, 7], [8, 8]]],
  [15, [[0, 2], [1, 2], [2, 2]]],
  [18, [[8, 2], [7, 3], [8, 3]]],
  [15, [[4, 5], [5, 5], [4, 6]]],
  [12, [[2, 6], [2, 7], [1, 7]]],
  [14, [[3, 6], [3, 7], [4, 7]]],
  [16, [[5, 6], [5, 7], [5, 8]]],
  [18, [[2, 8], [3, 8], [4, 8]]],
  [25, [[6, 1], [7, 1], [8, 1], [7, 2]]],
  [10, [[5, 2], [5, 3], [5, 4], [4, 4]]],
  [17, [[0, 3], [0, 4], [1, 4], [1, 5]]],
  [15, [[8, 5], [8, 6], [7, 6], [6, 6]]],
  [27, [[0, 5], [0, 6], [0, 7], [1, 6]]],
];

const EXAMPLE_CAGES: Cage[] = EXAMPLE_CAGES_RAW.map(([sum, cells], i) => ({
  sum,
  id: i,
  cells: cells.map(([row, col]) => ({ row, col })),
}));

export class SudokuEngine {
  private module: {
    [key: string]: (...args: unknown[]) => unknown;
  } | undefined;

  public readonly onUpdate = new Set<() => void>();

  constructor() {
  }

  public async init(): Promise<void> {
    if (this.module) return;
    this.module = await createSudokuModule();
    console.log("Wasm ready", this.module);
  }

  public getBoard(): BoardState {
    const board: BoardState = [];
    for (let row = 0; row < 9; row++) {
      const rowCells: CellState[] = [];
      for (let col = 0; col < 9; col++) {
        const value = this.module!._getBoardValue(row, col) as number;
        const cage = EXAMPLE_CAGES.find(c =>
          c.cells.some(p => p.row == row && p.col == col),
        );
        rowCells.push({ cage, value: value || null, domain: new Set() });
      }
      board.push(rowCells);
    }
    return board;
  }

  public getCages(): readonly Cage[] {
    return EXAMPLE_CAGES;
  }

  // Placeholder for solver integration
  public async solve(): Promise<boolean> {
    await Promise.resolve();
    console.log("Solving...");
    const success = this.module!._runSolver();
    console.log("Solver returned:", success);
    this.update();
    return !!success;
  }

  public async step(): Promise<boolean> {
    console.log("Stepping...");
    // TODO: Call WASM solver step
    this.update();
    return true;
  }

  public reset(forward: boolean, mrv: boolean, lcv: boolean) {
    this.module!._initGame(forward, mrv, lcv);
    console.log("Game reset.");
    this.update();
  }

  private update() {
    this.onUpdate.forEach(f => void f());
  }
}

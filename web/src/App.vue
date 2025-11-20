<script setup lang="ts">
import { ref } from "vue";
import SudokuDomain from "./components/SudokuDomain.vue";
import SudokuGrid from "./components/SudokuGrid.vue";
import ControlPanel from "./components/ControlPanel.vue";
import { SudokuEngine } from "./lib/sudoku";

const logoDomain = ref(new Set([1, 3, 5]));

setInterval(() => {
  const randVal = () => Math.ceil(Math.random() * 9);
  logoDomain.value = new Set([randVal(), randVal(), randVal(), randVal()]);
}, 1200);

const engine = new SudokuEngine();
const isSolving = ref(false);

function loadSample() {
    engine.reset();
    
    // Example Killer Sudoku setup (simplified)
    // Just setting some cages and a few initial values if any
    
    // Cage 1: Sum 3 (1, 2) at (0,0), (0,1)
    engine.addCage({
        id: 'c1',
        sum: 3,
        cells: [{row: 0, col: 0}, {row: 0, col: 1}]
    });
    
    // Cage 2: Sum 15 (0,2), (0,3), (1,2)
    engine.addCage({
        id: 'c2',
        sum: 15,
        cells: [{row: 0, col: 2}, {row: 0, col: 3}, {row: 1, col: 2}]
    });

    // Some initial values (standard sudoku clues)
    const values: (number | null)[][] = Array.from({ length: 9 }, () => Array(9).fill(null));
    
    engine.loadPuzzle(values);
}

async function solve() {
    isSolving.value = true;
    await engine.solve();
    isSolving.value = false;
}

async function step() {
    isSolving.value = true;
    await engine.step();
    isSolving.value = false;
}

function reset() {
    engine.reset();
}

</script>

<template>
  <header>
    <div class="flex flex-col items-center gap-4 my-8">
      <div class="flex items-center gap-4">
          <SudokuDomain
            :domain="logoDomain"
            class="size-16"
          />
          <h1 class="text-4xl font-bold text-gray-800">Killer Sudoku Solver</h1>
      </div>
    </div>
  </header>

  <main class="flex flex-col items-center gap-8 pb-12">
    <ControlPanel 
        :solving="isSolving"
        @load-sample="loadSample"
        @solve="solve"
        @step="step"
        @reset="reset"
    />
    
    <SudokuGrid :engine="engine" />
  </main>
</template>

<script setup lang="ts">
import { ref } from "vue";
import SudokuGrid from "./components/SudokuGrid.vue";
import ControlPanel from "./components/ControlPanel.vue";
import { SudokuEngine } from "./lib/sudoku";

const engine = new SudokuEngine();
const isSolving = ref(false);

function loadSample() {
    engine.reset();
    
    // Example Killer Sudoku setup (simplified)
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
  <div class="min-h-screen bg-slate-50 text-slate-900 font-sans selection:bg-indigo-100 selection:text-indigo-900">
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8 min-h-screen flex flex-col">
      
      <!-- Header -->
      <header class="flex items-center justify-between mb-12">
        <div class="flex items-center gap-3">
          <div class="size-10 bg-indigo-600 rounded-lg flex items-center justify-center shadow-lg shadow-indigo-200">
            <span class="text-white font-bold text-xl">K</span>
          </div>
          <h1 class="text-2xl font-bold tracking-tight text-slate-900">Killer Sudoku <span class="text-indigo-600">Solver</span></h1>
        </div>
        <div class="text-sm font-medium text-slate-500">
          v1.0.1
        </div>
      </header>

      <!-- Main Content -->
      <main class="flex-1 flex flex-col lg:flex-row gap-12 items-start justify-center">
        
        <!-- Left Column: Controls & Info -->
        <div class="w-full lg:w-80 flex flex-col gap-6 order-2 lg:order-1">
          <div class="bg-white rounded-2xl p-6 shadow-sm border border-slate-100">
            <h2 class="text-lg font-semibold mb-4 text-slate-800">Controls</h2>
            <ControlPanel 
                :solving="isSolving"
                @load-sample="loadSample"
                @solve="solve"
                @step="step"
                @reset="reset"
            />
          </div>

          <div class="bg-indigo-50 rounded-2xl p-6 border border-indigo-100">
            <h3 class="text-indigo-900 font-semibold mb-2">How it works</h3>
            <p class="text-indigo-700/80 text-sm leading-relaxed">
              This visualizer demonstrates the constraint satisfaction algorithm. 
              Watch as the engine propagates constraints through cages and rows.
            </p>
          </div>
        </div>

        <!-- Right Column: The Grid -->
        <div class="flex-1 w-full max-w-2xl order-1 lg:order-2 flex justify-center">
           <div class="w-full bg-white p-2 rounded-xl shadow-xl shadow-slate-200/50 border border-slate-100">
              <SudokuGrid :engine="engine" />
           </div>
        </div>

      </main>
    </div>
  </div>
</template>

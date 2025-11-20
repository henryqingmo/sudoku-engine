<script setup lang="ts">
import { onMounted, ref, watch } from "vue";
import ControlPanel from "./components/ControlPanel.vue";
import SudokuGrid from "./components/SudokuGrid.vue";
import { SudokuEngine } from "./lib/sudoku";

const engine = new SudokuEngine();
const isSolving = ref(false);

const heuristics = ref({
  forward: false,
  mrv: false,
  lcv: false,
});

watch(heuristics, (heuristics) => {
  engine.reset(heuristics.forward, heuristics.mrv, heuristics.lcv);
});

async function solve() {
  reset();
  isSolving.value = true;
  await engine.solve();
  isSolving.value = false;
}

async function stepRaw() {
  await engine.step();
  if (heuristics.value.forward) {
    // idek why :')
    await engine.step();
  }
};

async function step() {
  isSolving.value = true;
  await stepRaw();
  isSolving.value = false;
}

let autoStepTimeout: number;

async function auto() {
  isSolving.value = true;
  let promise = Promise.resolve();
  autoStepTimeout = setInterval(() => {
    promise.then(() => promise = stepRaw());
  }, 100);
}

function pause() {
  if (!!autoStepTimeout) {
    clearInterval(autoStepTimeout);
    isSolving.value = false;
  }
}

function reset() {
  engine.reset(
    heuristics.value.forward,
    heuristics.value.mrv,
    heuristics.value.lcv,
  );
}

const engineReady = ref(false);

onMounted(() => {
  engine.init()
    .then(() => engineReady.value = true)
    .then(reset);
});

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
          <h1 class="text-2xl font-bold tracking-tight text-slate-900">Killer Sudoku <span
              class="text-indigo-600">Solver</span></h1>
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
              v-model="heuristics"
              :solving="isSolving"
              @solve="solve"
              @step="step"
              @auto="auto"
              @pause="pause"
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
            <SudokuGrid
              v-if="engineReady"
              :engine
            />
          </div>
        </div>

      </main>
    </div>
  </div>
</template>

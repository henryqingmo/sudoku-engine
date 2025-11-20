<script setup lang="ts">
import { computed, ref } from 'vue';
import { SudokuEngine, type CellState, type Cage } from '../lib/sudoku';
import SudokuCell from './SudokuCell.vue';

const props = defineProps<{
  engine: SudokuEngine;
}>();

const board = computed(() => props.engine.getBoard());
const cages = computed(() => props.engine.getCages());

const selectedCell = ref<{r: number, c: number} | null>(null);

function isCageTopLeft(r: number, c: number, cageId?: string): boolean {
  if (!cageId) return false;
  const cage = props.engine.getCage(cageId);
  if (!cage) return false;
  
  // Find top-left most cell in cage
  let minR = 9, minC = 9;
  for (const cell of cage.cells) {
    if (cell.row < minR) {
      minR = cell.row;
      minC = cell.col;
    } else if (cell.row === minR && cell.col < minC) {
      minC = cell.col;
    }
  }
  return r === minR && c === minC;
}

function getCage(cageId?: string): Cage | undefined {
    if (!cageId) return undefined;
    return props.engine.getCage(cageId);
}

function onCellClick(r: number, c: number) {
    if (selectedCell.value?.r === r && selectedCell.value?.c === c) {
        selectedCell.value = null;
    } else {
        selectedCell.value = { r, c };
    }
}

</script>

<template>
  <div class="relative w-full aspect-square bg-slate-800 rounded-lg p-1 shadow-inner">
    <!-- The Grid Container -->
    <div class="grid grid-cols-9 grid-rows-9 gap-px bg-slate-300 w-full h-full border-2 border-slate-800 rounded overflow-hidden">
      <template v-for="(row, r) in board" :key="r">
        <template v-for="(cell, c) in row" :key="`${r}-${c}`">
          <div 
            class="relative bg-white overflow-hidden"
            :class="{
                // Thicker borders for 3x3 blocks using margin tricks or specific borders
                // Actually, using gap-px gives us 1px borders everywhere.
                // We can add extra borders to specific cells to simulate the 3x3 grid.
                'border-r-2 border-r-slate-800': (c + 1) % 3 === 0 && c !== 8,
                'border-b-2 border-b-slate-800': (r + 1) % 3 === 0 && r !== 8,
            }"
          >
            <SudokuCell
              :cell="cell"
              :row="r"
              :col="c"
              :cage="getCage(cell.cageId)"
              :is-cage-top-left="isCageTopLeft(r, c, cell.cageId)"
              :selected="selectedCell?.r === r && selectedCell?.c === c"
              @update:selected="onCellClick(r, c)"
            />
          </div>
        </template>
      </template>
    </div>
  </div>
</template>

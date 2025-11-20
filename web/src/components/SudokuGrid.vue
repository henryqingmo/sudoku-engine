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
  <div class="relative w-full max-w-2xl aspect-square border-4 border-gray-800">
    <div class="grid grid-cols-9 grid-rows-9 gap-0 bg-gray-800 w-full h-full">
      <template v-for="(row, r) in board" :key="r">
        <template v-for="(cell, c) in row" :key="`${r}-${c}`">
          <div 
            class="relative bg-white"
            :class="{
                'border-r-2 border-gray-800': (c + 1) % 3 === 0 && c !== 8,
                'border-b-2 border-gray-800': (r + 1) % 3 === 0 && r !== 8,
                'border-r border-gray-800': (c + 1) % 3 !== 0,
                'border-b border-gray-800': (r + 1) % 3 !== 0
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
            
            <!-- Cage Borders Overlay (Simplified) -->
            <!-- If we want true cage borders, we'd need more complex logic here or SVG overlay -->
            <div v-if="cell.cageId" class="absolute inset-0 pointer-events-none border-dashed border-gray-400/50"
                 :class="{
                     'border-2': true
                 }"
            ></div>
          </div>
        </template>
      </template>
    </div>
  </div>
</template>

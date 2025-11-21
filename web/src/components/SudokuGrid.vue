<script setup lang="ts">
import { onMounted, onUnmounted, ref } from "vue";
import { SudokuEngine, type CellState } from "../lib/sudoku";
import SudokuCell from "./SudokuCell.vue";

const props = defineProps<{
  engine: SudokuEngine;
}>();

const board = ref(props.engine.getBoard());

const selectedCell = ref<{ r: number; c: number } | null>(null);

function onCellClick(r: number, c: number) {
  if (selectedCell.value?.r === r && selectedCell.value?.c === c) {
    selectedCell.value = null;
  }
  else {
    selectedCell.value = { r, c };
  }
}

function isSameCage(cell: CellState, r2: number, c2: number): boolean {
  if (r2 < 0 || r2 >= 9 || c2 < 0 || c2 >= 9) return false;
  const other = board.value[r2]?.[c2];
  if (!other) return false;
  return !!cell.cage && !!other.cage && cell.cage.id === other.cage.id;
}

function updateBoard() {
  // console.debug("Board updated.");
  board.value = props.engine.getBoard();
}

onMounted(() => props.engine.onUpdate.add(updateBoard));
onUnmounted(() => props.engine.onUpdate.delete(updateBoard));

</script>

<template>
  <div class="relative w-full aspect-square bg-slate-300 rounded-lg p-1 shadow-inner">
    <!-- The Grid Container -->
    <div
      class="grid grid-cols-9 grid-rows-9 gap-px bg-slate-300 w-full h-full border-2 border-slate-800 rounded overflow-hidden"
    >
      <template
        v-for="(row, r) in board"
        :key="r"
      >
        <template
          v-for="(cell, c) in row"
          :key="`${r}-${c}`"
        >
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
              :selected="selectedCell?.r === r && selectedCell?.c === c"
              :same-cage-top="isSameCage(cell, r - 1, c)"
              :same-cage-bottom="isSameCage(cell, r + 1, c)"
              :same-cage-left="isSameCage(cell, r, c - 1)"
              :same-cage-right="isSameCage(cell, r, c + 1)"
              @update:selected="onCellClick(r, c)"
            />
          </div>
        </template>
      </template>
    </div>
  </div>
</template>

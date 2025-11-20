<script setup lang="ts">
import { computed } from "vue";
import type { Cage, CellState } from "../lib/sudoku";
import SudokuDomain from "./SudokuDomain.vue";

const props = defineProps<{
  cell: CellState;
  row: number;
  col: number;
  sameCageTop?: boolean;
  sameCageBottom?: boolean;
  sameCageLeft?: boolean;
  sameCageRight?: boolean;
}>();

const isSelected = defineModel<boolean>("selected");

const cellClasses = computed(() => {
  return {
    "relative flex items-center justify-center select-none cursor-pointer transition-colors duration-150": true,
    "bg-white hover:bg-slate-50": !isSelected.value,
    "bg-indigo-100 text-indigo-900": isSelected.value,
    "text-3xl font-medium": props.cell.value !== null,
    "text-slate-700": props.cell.value !== null && !isSelected.value,
  };
});

// Cage styling
const cageClasses = computed(() => {
  if (!props.cell.cage) return {};
  return {
    // Use a subtle dashed border for cages
    "after:absolute after:border-dashed after:border-indigo-300/80 after:pointer-events-none": true,

    // Borders: Only draw border if neighbor is NOT in same cage
    "after:border-t-2": !props.sameCageTop,
    "after:border-b-2": !props.sameCageBottom,
    "after:border-l-2": !props.sameCageLeft,
    "after:border-r-2": !props.sameCageRight,

    // Insets: If neighbor is in same cage, extend to edge (0). Else leave gap (0.5 = 2px)
    "after:top-0.5": !props.sameCageTop,
    "after:top-0": props.sameCageTop,
    "after:bottom-0.5": !props.sameCageBottom,
    "after:bottom-0": props.sameCageBottom,
    "after:left-0.5": !props.sameCageLeft,
    "after:left-0": props.sameCageLeft,
    "after:right-0.5": !props.sameCageRight,
    "after:right-0": props.sameCageRight,

    // Rounded corners for the cage "blob"
    // This is tricky with individual borders.
    // We can try to round corners where borders meet.
    // But standard rounded-sm might look okay if we have gaps.
    "after:rounded-sm": !props.sameCageTop && !props.sameCageLeft, // Top-left corner of cage

    // And a very subtle background tint
    "bg-indigo-50/30": !isSelected.value,
  };
});

const isCageTopLeft = computed<boolean>(() =>
  ((r: number, c: number, cage?: Cage): boolean => {
    if (!cage) return false;

    // Find top-left most cell in cage
    let minR = 9, minC = 9;
    for (const cell of cage.cells) {
      if (cell.row < minR) {
        minR = cell.row;
        minC = cell.col;
      }
      else if (cell.row === minR && cell.col < minC) {
        minC = cell.col;
      }
    }
    return r === minR && c === minC;
  })(props.row, props.col, props.cell.cage),
);

</script>

<template>
  <div
    :class="[cellClasses, cageClasses]"
    @click="isSelected = !isSelected"
    class="aspect-square w-full h-full"
  >
    <!-- Cage Sum Indicator -->
    <div
      v-if="isCageTopLeft && cell.cage"
      class="absolute top-0.5 left-1 text-[15px] font-bold text-indigo-500/80 z-10 bg-white/80 px-0.5 rounded"
    >
      {{ cell.cage.sum }}
    </div>

    <!-- Value -->
    <span v-if="cell.value !== null">
      {{ cell.value }}
    </span>

    <!-- Domain/Candidates -->
    <div
      v-else
      class="w-full h-full p-0.5"
    >
      <SudokuDomain
        :domain="cell.domain"
        class="w-full h-full"
      />
    </div>
  </div>
</template>

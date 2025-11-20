<script setup lang="ts">
import { computed } from "vue";
import type { CellState, Cage } from "../lib/sudoku";
import SudokuDomain from "./SudokuDomain.vue";

const props = defineProps<{
  cell: CellState;
  row: number;
  col: number;
  cage?: Cage;
  isCageTopLeft?: boolean;
}>();

const isSelected = defineModel<boolean>("selected");

const cellClasses = computed(() => {
  return {
    "relative flex items-center justify-center select-none cursor-pointer transition-colors duration-150": true,
    "bg-white hover:bg-slate-50": !props.cell.isFixed && !isSelected.value,
    "bg-slate-100 text-slate-900": props.cell.isFixed,
    "bg-indigo-100 text-indigo-900": isSelected.value,
    "text-3xl font-medium": props.cell.value !== null,
    "text-slate-700": props.cell.value !== null && !props.cell.isFixed && !isSelected.value,
  };
});

// Cage styling
const cageClasses = computed(() => {
  if (!props.cage) return {};
  return {
    // Use a subtle dashed border for cages
    "after:absolute after:inset-0.5 after:border-2 after:border-dashed after:border-indigo-200/70 after:rounded-sm after:pointer-events-none": true,
    // And a very subtle background tint
    "bg-indigo-50/30": !isSelected.value && !props.cell.isFixed,
  };
});

</script>

<template>
  <div
    :class="[cellClasses, cageClasses]"
    @click="isSelected = !isSelected"
    class="aspect-square w-full h-full"
  >
    <!-- Cage Sum Indicator -->
    <div v-if="isCageTopLeft && cage" class="absolute top-0.5 left-1 text-[15px] font-bold text-indigo-500/80 z-10 bg-white/80 px-0.5 rounded">
      {{ cage.sum }}
    </div>

    <!-- Value -->
    <span v-if="cell.value !== null">
      {{ cell.value }}
    </span>

    <!-- Domain/Candidates -->
    <div v-else class="w-full h-full p-0.5">
        <SudokuDomain :domain="cell.domain" class="w-full h-full" />
    </div>
  </div>
</template>

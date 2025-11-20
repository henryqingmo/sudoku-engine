<script setup lang="ts">
import { computed } from 'vue';
import type { CellState, Cage } from '../lib/sudoku';
import SudokuDomain from './SudokuDomain.vue';

const props = defineProps<{
  cell: CellState;
  row: number;
  col: number;
  cage?: Cage;
  isCageTopLeft?: boolean;
}>();

const isSelected = defineModel<boolean>('selected');

const cellClasses = computed(() => {
  return {
    'relative flex items-center justify-center border border-gray-300 select-none cursor-pointer': true,
    'bg-white': !props.cell.isFixed && !isSelected.value,
    'bg-gray-100': props.cell.isFixed,
    'bg-blue-100': isSelected.value,
    'text-3xl font-bold text-gray-800': props.cell.value !== null,
  };
});

// Cage styling
// We can use inline styles or dynamic classes for cage borders if we want to be fancy.
// For now, let's assume the grid handles the heavy lifting of cage borders, 
// or we just use a dashed outline if it's in a cage.
// Actually, a common way is to color the background of cages slightly differently or use a dashed border.
// Since we are using Tailwind, we can try to apply classes based on cage presence.

const cageClasses = computed(() => {
    if (!props.cage) return {};
    // This is a simplification. Ideally we want borders only on the edges of the cage.
    // That requires knowing neighbors.
    // For this minimal version, let's just show the cage ID or sum if it's top left.
    return {
        'bg-yellow-50/50': true // Light highlight for caged cells
    }
});

</script>

<template>
  <div 
    :class="[cellClasses, cageClasses]"
    @click="isSelected = !isSelected"
    class="aspect-square w-full h-full"
  >
    <!-- Cage Sum Indicator -->
    <div v-if="isCageTopLeft && cage" class="absolute top-0 left-1 text-xs font-bold text-gray-700 z-10">
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

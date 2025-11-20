<script setup lang="ts">
import { ref } from "vue";
import SudokuDomain from "./components/SudokuDomain.vue";

const logoDomain = ref(new Set([1, 3, 5]));

setInterval(() => {
  const randVal = () => Math.ceil(Math.random() * 9);
  logoDomain.value = new Set([randVal(), randVal(), randVal(), randVal()]);
}, 1200);
</script>

<template>
  <header>
    <div class="flex flex-col items-center gap-4 my-8">
      <SudokuDomain
        :domain="logoDomain"
        class="size-16"
      />
      <h1 class="text-4xl">Sudoku Solver</h1>
    </div>
  </header>

  <main>
    <!-- TODO -->
  </main>
</template>

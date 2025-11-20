<script setup lang="ts">
defineProps<{
  domain: Set<number>;
}>();
</script>

<template>
  <div class="grid grid-cols-3 grid-rows-3">
    <div v-for="value in 9" :key="value" :class="{
      'bg-neutral-500/60 transition-opacity': true,
      'opacity-40': !domain.has(value)
    }" />
  </div>
</template>

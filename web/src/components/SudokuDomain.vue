<script setup lang="ts">
defineProps<{
  domain: Set<number>;
}>();
</script>

<template>
  <div class="grid grid-cols-3 grid-rows-3 w-full h-full p-1 gap-px">
    <div v-for="value in 9" :key="value" 
      class="flex items-center justify-center text-[8px] font-medium text-slate-400 select-none rounded-sm"
      :class="{
        'opacity-0': !domain.has(value),
        'bg-slate-100 text-slate-500': domain.has(value)
      }"
    >
      {{ value }}
    </div>
  </div>
</template>

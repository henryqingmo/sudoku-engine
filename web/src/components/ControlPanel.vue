<script setup lang="ts">
defineProps<{
  solving: boolean;
}>();

const emit = defineEmits<{
  (e: 'load-sample'): void;
  (e: 'solve'): void;
  (e: 'step'): void;
  (e: 'reset'): void;
}>();
</script>

<template>
  <div class="flex flex-wrap gap-4 p-4 bg-gray-50 rounded-lg shadow-sm">
    <button 
      @click="emit('load-sample')"
      class="px-4 py-2 bg-cyan-600 text-white rounded hover:bg-cyan-700 transition-colors"
      :disabled="solving"
    >
      Load Sample
    </button>
    
    <button 
      @click="emit('step')"
      class="px-4 py-2 bg-lime-600 text-white rounded hover:bg-lime-700 transition-colors"
      :disabled="solving"
    >
      Step
    </button>

    <button 
      @click="emit('solve')"
      class="px-4 py-2 bg-purple-600 text-white rounded hover:bg-purple-800 transition-colors"
      :disabled="solving"
    >
      Solve All
    </button>

    <button 
      @click="emit('reset')"
      class="px-4 py-2 bg-red-600 text-white rounded hover:bg-red-800 transition-colors"
    >
      Reset
    </button>
  </div>
</template>

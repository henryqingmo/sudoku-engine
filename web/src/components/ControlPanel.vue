<script setup lang="ts">
defineProps<{
  solving: boolean;
}>();

const model = defineModel<{
  forward: boolean;
  mrv: boolean;
  lcv: boolean;
}>({ required: true });

const emit = defineEmits<{
  (e: "load-sample"): void;
  (e: "solve"): void;
  (e: "step"): void;
  (e: "auto"): void;
  (e: "pause"): void;
  (e: "reset"): void;
}>();
</script>

<template>
  <div class="flex flex-col gap-3 w-full">
    <!-- <button
      @click="emit('load-sample')"
      class="w-full px-4 py-3 bg-indigo-600 text-white font-medium rounded-xl hover:bg-indigo-700 active:bg-indigo-800 transition-all shadow-sm hover:shadow-md disabled:opacity-50 disabled:cursor-not-allowed flex items-center justify-center gap-2"
      :disabled="solving"
    >
      <span class="text-lg"></span> Load Sample
    </button> -->

    <div class="grid grid-cols-2 gap-3">

      <button
        @click="emit('step')"
        class="px-4 py-3 bg-indigo-500 text-white font-medium rounded-xl hover:bg-indigo-300 active:bg-indigo-700 transition-all shadow-sm hover:shadow-md disabled:opacity-50 disabled:cursor-not-allowed"
        :disabled="solving"
      >
        Step
      </button>

      <button
        @click="emit('solve')"
        class="px-4 py-3 bg-emerald-500 text-white font-medium rounded-xl hover:bg-emerald-600 active:bg-emerald-700 transition-all shadow-sm hover:shadow-md disabled:opacity-50 disabled:cursor-not-allowed"
        :disabled="solving"
      >
        Solve All
      </button>
    </div>

    <div class="grid grid-cols-2 gap-3">
      <button
        @click="emit('auto')"
        class="px-4 py-3 bg-white text-slate-700 font-medium rounded-xl border border-slate-200 hover:bg-slate-50 hover:border-slate-300 active:bg-slate-100 transition-all disabled:opacity-50 disabled:cursor-not-allowed"
        :disabled="solving"
      >
        Auto Step
      </button>

      <button
        @click="emit('pause')"
        class="px-4 py-3 bg-white-50 text-slate-700 font-medium rounded-xl border border-slate-200 hover:bg-slate-50 hover:border-slate-300 active:bg-slate-100 transition-all disabled:opacity-50 disabled:cursor-not-allowed"
        :disabled="!solving"
      >
        Pause
      </button>
    </div>

    <div>
      <div class="flex items-center gap-3">
        <input
          type="checkbox"
          v-model="model.forward"
        />
        Forward Checking
      </div>
      <div
        v-if="model.forward"
        class="flex items-center gap-3"
      >
        <input
          type="checkbox"
          v-model="model.mrv"
        />
        MRV
        <input
          type="checkbox"
          v-model="model.lcv"
        />
        LCV
      </div>
    </div>

    <button
      @click="emit('reset')"
      class="w-full px-4 py-3 text-rose-600 font-medium rounded-xl hover:bg-rose-50 transition-colors mt-2"
    >
      Reset Board
    </button>
  </div>
</template>

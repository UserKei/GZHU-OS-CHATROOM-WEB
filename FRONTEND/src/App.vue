<template>
  <div id="app" :class="{ dark: isDarkMode }">
    <router-view />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useDark, useToggle } from '@vueuse/core'

// Dark mode support
const isDarkMode = useDark()
const toggleDark = useToggle(isDarkMode)

onMounted(() => {
  // Initialize app
  console.log('Chat Room App Initialized')
})
</script>

<style scoped>
#app {
  min-height: 100vh;
  transition: all 0.3s ease;
}
</style>

import { ref, computed } from 'vue'
import { defineStore } from 'pinia'

export const useAppStore = defineStore('app', () => {
  const isLoading = ref(false)
  const theme = ref<'light' | 'dark' | 'auto'>('auto')
  const sidebarCollapsed = ref(false)

  const isDarkTheme = computed(() => {
    if (theme.value === 'auto') {
      return window.matchMedia('(prefers-color-scheme: dark)').matches
    }
    return theme.value === 'dark'
  })

  function setLoading(loading: boolean) {
    isLoading.value = loading
  }

  function setTheme(newTheme: 'light' | 'dark' | 'auto') {
    theme.value = newTheme
  }

  function toggleSidebar() {
    sidebarCollapsed.value = !sidebarCollapsed.value
  }

  return {
    isLoading,
    theme,
    sidebarCollapsed,
    isDarkTheme,
    setLoading,
    setTheme,
    toggleSidebar
  }
})

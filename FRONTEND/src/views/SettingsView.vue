<template>
  <div class="min-h-screen bg-apple-light-gray dark:bg-apple-dark-gray">
    <!-- Header -->
    <div class="bg-white dark:bg-gray-800 border-b border-gray-200 dark:border-gray-700 p-6">
      <div class="max-w-4xl mx-auto flex items-center justify-between">
        <div class="flex items-center space-x-4">
          <el-button
            circle
            @click="router.back()"
            class="!border-gray-300 dark:!border-gray-600"
          >
            <el-icon><ArrowLeft /></el-icon>
          </el-button>
          <h1 class="text-2xl font-bold gradient-text">Settings</h1>
        </div>
      </div>
    </div>

    <div class="max-w-4xl mx-auto p-6 space-y-6">
      <!-- Appearance Settings -->
      <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
        <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-6 flex items-center">
          <el-icon class="mr-3 text-apple-blue"><Sunny /></el-icon>
          Appearance
        </h3>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <!-- Theme Selection -->
          <div class="space-y-4">
            <h4 class="font-medium text-gray-900 dark:text-white">Theme</h4>
            <div class="space-y-2">
              <div
                v-for="theme in themes"
                :key="theme.value"
                class="flex items-center p-3 rounded-2xl cursor-pointer transition-colors"
                :class="currentTheme === theme.value ? 'bg-apple-blue text-white' : 'bg-apple-light-gray dark:bg-gray-700 hover:bg-gray-100 dark:hover:bg-gray-600'"
                @click="setTheme(theme.value)"
              >
                <el-icon class="mr-3">
                  <component :is="theme.icon" />
                </el-icon>
                <div>
                  <div class="font-medium">{{ theme.name }}</div>
                  <div class="text-sm opacity-75">{{ theme.description }}</div>
                </div>
              </div>
            </div>
          </div>

          <!-- Font Size -->
          <div class="space-y-4">
            <h4 class="font-medium text-gray-900 dark:text-white">Font Size</h4>
            <div class="px-4">
              <el-slider
                v-model="fontSize"
                :min="12"
                :max="20"
                :step="1"
                @change="updateFontSize"
                class="mb-4"
              />
              <div class="text-center text-gray-500 dark:text-gray-400">
                {{ fontSize }}px
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Notification Settings -->
      <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
        <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-6 flex items-center">
          <el-icon class="mr-3 text-apple-green"><Bell /></el-icon>
          Notifications
        </h3>

        <div class="space-y-4">
          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Desktop Notifications</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Show notifications on desktop</p>
            </div>
            <el-switch v-model="notificationSettings.desktop" @change="updateNotificationSettings" />
          </div>

          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Sound Notifications</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Play sound for new messages</p>
            </div>
            <el-switch v-model="notificationSettings.sound" @change="updateNotificationSettings" />
          </div>

          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Mention Notifications</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Only notify when mentioned</p>
            </div>
            <el-switch v-model="notificationSettings.mentionOnly" @change="updateNotificationSettings" />
          </div>
        </div>
      </div>

      <!-- Chat Settings -->
      <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
        <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-6 flex items-center">
          <el-icon class="mr-3 text-telegram-blue"><ChatDotRound /></el-icon>
          Chat Settings
        </h3>

        <div class="space-y-4">
          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Auto-scroll to Bottom</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Automatically scroll to new messages</p>
            </div>
            <el-switch v-model="chatSettings.autoScroll" @change="updateChatSettings" />
          </div>

          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Show Timestamps</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Display message timestamps</p>
            </div>
            <el-switch v-model="chatSettings.showTimestamps" @change="updateChatSettings" />
          </div>

          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Compact Mode</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Reduce spacing between messages</p>
            </div>
            <el-switch v-model="chatSettings.compactMode" @change="updateChatSettings" />
          </div>

          <div class="p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <h4 class="font-medium text-gray-900 dark:text-white mb-3">Message History</h4>
            <p class="text-sm text-gray-500 dark:text-gray-400 mb-4">Keep messages for</p>
            <el-select
              v-model="chatSettings.historyDuration"
              @change="updateChatSettings"
              class="w-full"
            >
              <el-option label="1 Day" value="1" />
              <el-option label="3 Days" value="3" />
              <el-option label="1 Week" value="7" />
              <el-option label="1 Month" value="30" />
              <el-option label="Forever" value="0" />
            </el-select>
          </div>
        </div>
      </div>

      <!-- Privacy & Security -->
      <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
        <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-6 flex items-center">
          <el-icon class="mr-3 text-apple-purple"><Lock /></el-icon>
          Privacy & Security
        </h3>

        <div class="space-y-4">
          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Show Online Status</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Let others see when you're online</p>
            </div>
            <el-switch v-model="privacySettings.showOnlineStatus" @change="updatePrivacySettings" />
          </div>

          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Read Receipts</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Show when you've read messages</p>
            </div>
            <el-switch v-model="privacySettings.readReceipts" @change="updatePrivacySettings" />
          </div>

          <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <div>
              <h4 class="font-medium text-gray-900 dark:text-white">Data Collection</h4>
              <p class="text-sm text-gray-500 dark:text-gray-400">Allow analytics for improving service</p>
            </div>
            <el-switch v-model="privacySettings.dataCollection" @change="updatePrivacySettings" />
          </div>

          <div class="p-4 bg-yellow-50 dark:bg-yellow-900/20 rounded-2xl border border-yellow-200 dark:border-yellow-800">
            <div class="flex items-center justify-between">
              <div>
                <h4 class="font-medium text-yellow-800 dark:text-yellow-200">Clear Chat History</h4>
                <p class="text-sm text-yellow-600 dark:text-yellow-300">Delete all your messages permanently</p>
              </div>
              <el-button
                type="warning"
                size="small"
                @click="clearChatHistory"
                class="!bg-yellow-600 !border-yellow-600"
              >
                Clear
              </el-button>
            </div>
          </div>
        </div>
      </div>

      <!-- Advanced Settings -->
      <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
        <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-6 flex items-center">
          <el-icon class="mr-3 text-apple-orange"><Setting /></el-icon>
          Advanced
        </h3>

        <div class="space-y-4">
          <div class="p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <h4 class="font-medium text-gray-900 dark:text-white mb-3">Server Connection</h4>
            <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <label class="block text-sm text-gray-500 dark:text-gray-400 mb-1">WebSocket URL</label>
                <el-input
                  v-model="serverSettings.wsUrl"
                  placeholder="ws://localhost:8080/ws"
                  @change="updateServerSettings"
                />
              </div>
              <div>
                <label class="block text-sm text-gray-500 dark:text-gray-400 mb-1">Reconnect Interval (seconds)</label>
                <el-input-number
                  v-model="serverSettings.reconnectInterval"
                  :min="1"
                  :max="60"
                  @change="updateServerSettings"
                />
              </div>
            </div>
          </div>

          <div class="p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
            <h4 class="font-medium text-gray-900 dark:text-white mb-3">Debug Mode</h4>
            <div class="flex items-center justify-between">
              <div>
                <p class="text-sm text-gray-500 dark:text-gray-400">Enable debug logging</p>
              </div>
              <el-switch v-model="debugMode" @change="updateDebugMode" />
            </div>
          </div>

          <div class="p-4 bg-red-50 dark:bg-red-900/20 rounded-2xl border border-red-200 dark:border-red-800">
            <div class="flex items-center justify-between">
              <div>
                <h4 class="font-medium text-red-600 dark:text-red-400">Reset Settings</h4>
                <p class="text-sm text-red-500 dark:text-red-400">Reset all settings to default</p>
              </div>
              <el-button
                type="danger"
                size="small"
                @click="resetSettings"
                class="!bg-red-600 !border-red-600"
              >
                Reset
              </el-button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  ArrowLeft,
  Sunny,
  Moon,
  Monitor,
  Bell,
  ChatDotRound,
  Lock,
  Setting
} from '@element-plus/icons-vue'
import { useDark, useToggle } from '@vueuse/core'

const router = useRouter()
const isDarkMode = useDark()
const toggleDark = useToggle(isDarkMode)

// Theme settings
const themes = [
  { value: 'light', name: 'Light', description: 'Clean and bright', icon: Sunny },
  { value: 'dark', name: 'Dark', description: 'Easy on the eyes', icon: Moon },
  { value: 'auto', name: 'Auto', description: 'Follow system', icon: Monitor },
]

const currentTheme = ref('auto')
const fontSize = ref(14)

// Settings
const notificationSettings = reactive({
  desktop: true,
  sound: true,
  mentionOnly: false,
})

const chatSettings = reactive({
  autoScroll: true,
  showTimestamps: true,
  compactMode: false,
  historyDuration: '3',
})

const privacySettings = reactive({
  showOnlineStatus: true,
  readReceipts: true,
  dataCollection: false,
})

const serverSettings = reactive({
  wsUrl: 'ws://localhost:8080/ws',
  reconnectInterval: 3,
})

const debugMode = ref(false)

// Actions
const setTheme = (theme: string) => {
  currentTheme.value = theme

  if (theme === 'light') {
    isDarkMode.value = false
  } else if (theme === 'dark') {
    isDarkMode.value = true
  } else {
    // Auto mode - follow system preference
    const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)')
    isDarkMode.value = mediaQuery.matches
  }

  localStorage.setItem('theme', theme)
  ElMessage.success('Theme updated')
}

const updateFontSize = (size: number) => {
  document.documentElement.style.fontSize = `${size}px`
  localStorage.setItem('fontSize', size.toString())
  ElMessage.success('Font size updated')
}

const updateNotificationSettings = () => {
  localStorage.setItem('notificationSettings', JSON.stringify(notificationSettings))

  // Request notification permission if desktop notifications are enabled
  if (notificationSettings.desktop && Notification.permission === 'default') {
    Notification.requestPermission()
  }

  ElMessage.success('Notification settings updated')
}

const updateChatSettings = () => {
  localStorage.setItem('chatSettings', JSON.stringify(chatSettings))
  ElMessage.success('Chat settings updated')
}

const updatePrivacySettings = () => {
  localStorage.setItem('privacySettings', JSON.stringify(privacySettings))
  ElMessage.success('Privacy settings updated')
}

const updateServerSettings = () => {
  localStorage.setItem('serverSettings', JSON.stringify(serverSettings))
  ElMessage.success('Server settings updated')
}

const updateDebugMode = () => {
  localStorage.setItem('debugMode', debugMode.value.toString())

  if (debugMode.value) {
    console.log('Debug mode enabled')
  }

  ElMessage.success('Debug mode ' + (debugMode.value ? 'enabled' : 'disabled'))
}

const clearChatHistory = async () => {
  try {
    await ElMessageBox.confirm(
      'This will permanently delete all your chat messages. This action cannot be undone.',
      'Clear Chat History',
      {
        confirmButtonText: 'Clear All Messages',
        cancelButtonText: 'Cancel',
        type: 'warning',
      }
    )

    // TODO: Implement API call to clear chat history
    ElMessage.success('Chat history cleared')
  } catch {
    // User canceled
  }
}

const resetSettings = async () => {
  try {
    await ElMessageBox.confirm(
      'This will reset all settings to their default values.',
      'Reset Settings',
      {
        confirmButtonText: 'Reset',
        cancelButtonText: 'Cancel',
        type: 'warning',
      }
    )

    // Reset all settings to defaults
    currentTheme.value = 'auto'
    fontSize.value = 14

    Object.assign(notificationSettings, {
      desktop: true,
      sound: true,
      mentionOnly: false,
    })

    Object.assign(chatSettings, {
      autoScroll: true,
      showTimestamps: true,
      compactMode: false,
      historyDuration: '3',
    })

    Object.assign(privacySettings, {
      showOnlineStatus: true,
      readReceipts: true,
      dataCollection: false,
    })

    Object.assign(serverSettings, {
      wsUrl: 'ws://localhost:8080/ws',
      reconnectInterval: 3,
    })

    debugMode.value = false

    // Clear localStorage
    localStorage.removeItem('theme')
    localStorage.removeItem('fontSize')
    localStorage.removeItem('notificationSettings')
    localStorage.removeItem('chatSettings')
    localStorage.removeItem('privacySettings')
    localStorage.removeItem('serverSettings')
    localStorage.removeItem('debugMode')

    ElMessage.success('Settings reset to defaults')
  } catch {
    // User canceled
  }
}

// Load settings from localStorage
const loadSettings = () => {
  const savedTheme = localStorage.getItem('theme')
  if (savedTheme) {
    currentTheme.value = savedTheme
  }

  const savedFontSize = localStorage.getItem('fontSize')
  if (savedFontSize) {
    fontSize.value = parseInt(savedFontSize)
  }

  const savedNotificationSettings = localStorage.getItem('notificationSettings')
  if (savedNotificationSettings) {
    Object.assign(notificationSettings, JSON.parse(savedNotificationSettings))
  }

  const savedChatSettings = localStorage.getItem('chatSettings')
  if (savedChatSettings) {
    Object.assign(chatSettings, JSON.parse(savedChatSettings))
  }

  const savedPrivacySettings = localStorage.getItem('privacySettings')
  if (savedPrivacySettings) {
    Object.assign(privacySettings, JSON.parse(savedPrivacySettings))
  }

  const savedServerSettings = localStorage.getItem('serverSettings')
  if (savedServerSettings) {
    Object.assign(serverSettings, JSON.parse(savedServerSettings))
  }

  const savedDebugMode = localStorage.getItem('debugMode')
  if (savedDebugMode) {
    debugMode.value = savedDebugMode === 'true'
  }
}

onMounted(() => {
  loadSettings()
})
</script>

<style scoped>
.gradient-text {
  background: linear-gradient(135deg, #007AFF 0%, #2AABEE 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.shadow-apple {
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
}

.dark .shadow-apple {
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.2);
}
</style>

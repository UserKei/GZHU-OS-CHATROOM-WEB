<template>
  <div class="h-screen bg-apple-light-gray dark:bg-apple-dark-gray flex">
    <!-- Sidebar -->
    <div class="w-80 bg-white dark:bg-gray-800 border-r border-gray-200 dark:border-gray-700 flex flex-col">
      <!-- Header -->
      <div class="p-4 border-b border-gray-200 dark:border-gray-700">
        <div class="flex items-center justify-between mb-4">
          <h1 class="text-xl font-bold gradient-text">Chat Room</h1>
          <div class="flex items-center space-x-2">
            <!-- Dark Mode Toggle -->
            <el-button
              circle
              size="small"
              @click="toggleDark()"
              class="!border-none"
            >
              <el-icon>
                <Sunny v-if="isDarkMode" />
                <Moon v-else />
              </el-icon>
            </el-button>

            <!-- Settings -->
            <el-button
              circle
              size="small"
              @click="router.push('/settings')"
              class="!border-none"
            >
              <el-icon><Setting /></el-icon>
            </el-button>
          </div>
        </div>

        <!-- User Info -->
        <div class="flex items-center space-x-3">
          <div class="relative">
            <div
              class="user-avatar w-10 h-10"
              :style="{ backgroundColor: getUserColor(authStore.currentUser?.username || '') }"
            >
              {{ getInitials(authStore.currentUser?.username || '') }}
            </div>
            <div class="online-indicator"></div>
          </div>
          <div class="flex-1 min-w-0">
            <p class="font-semibold text-gray-900 dark:text-white truncate">
              {{ authStore.currentUser?.username }}
            </p>
            <p class="text-sm text-apple-green">Online</p>
          </div>
        </div>
      </div>

      <!-- Chat Rooms List -->
      <div class="flex-1 overflow-y-auto p-4">
        <div class="space-y-2">
          <div
            v-for="room in chatStore.rooms"
            :key="room.id"
            class="sidebar-item"
            :class="{ active: chatStore.activeRoomId === room.id }"
            @click="chatStore.setActiveRoom(room.id)"
          >
            <div class="flex items-center space-x-3 flex-1">
              <div
                class="user-avatar w-8 h-8 text-sm"
                :style="{ backgroundColor: getUserColor(room.name) }"
              >
                {{ getInitials(room.name) }}
              </div>
              <div class="flex-1 min-w-0">
                <p class="font-medium truncate">{{ room.name }}</p>
                <p class="text-sm text-gray-500 dark:text-gray-400 truncate">
                  {{ room.lastMessage?.content || 'No messages yet' }}
                </p>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Online Users -->
      <div class="p-4 border-t border-gray-200 dark:border-gray-700">
        <h3 class="text-sm font-semibold text-gray-500 dark:text-gray-400 mb-2">
          Online ({{ chatStore.onlineUsers.length }})
        </h3>
        <div class="flex flex-wrap gap-2">
          <div
            v-for="userId in chatStore.onlineUsers.slice(0, 6)"
            :key="userId"
            class="relative"
          >
            <div
              class="user-avatar w-6 h-6 text-xs"
              :style="{ backgroundColor: getUserColor(userId) }"
            >
              {{ getInitials(userId) }}
            </div>
          </div>
          <div
            v-if="chatStore.onlineUsers.length > 6"
            class="user-avatar w-6 h-6 text-xs bg-gray-400"
          >
            +{{ chatStore.onlineUsers.length - 6 }}
          </div>
        </div>
      </div>
    </div>

    <!-- Main Chat Area -->
    <div class="flex-1 flex flex-col">
      <!-- Chat Header -->
      <div class="p-4 bg-white dark:bg-gray-800 border-b border-gray-200 dark:border-gray-700">
        <div class="flex items-center justify-between">
          <div class="flex items-center space-x-3">
            <div
              class="user-avatar w-10 h-10"
              :style="{ backgroundColor: getUserColor(chatStore.activeRoom?.name || 'General') }"
            >
              {{ getInitials(chatStore.activeRoom?.name || 'General') }}
            </div>
            <div>
              <h2 class="font-semibold text-gray-900 dark:text-white">
                {{ chatStore.activeRoom?.name || 'General Chat' }}
              </h2>
              <p class="text-sm text-gray-500 dark:text-gray-400">
                {{ chatStore.onlineUsers.length }} members online
              </p>
            </div>
          </div>

          <div class="flex items-center space-x-2">
            <!-- Connection Status -->
            <div class="flex items-center space-x-2">
              <div
                class="w-2 h-2 rounded-full"
                :class="chatStore.isConnected ? 'bg-apple-green' : 'bg-apple-red'"
              ></div>
              <span class="text-sm text-gray-500 dark:text-gray-400">
                {{ chatStore.isConnected ? 'Connected' : 'Disconnected' }}
              </span>
            </div>

            <!-- User Menu -->
            <el-dropdown>
              <el-button circle size="small" class="!border-none">
                <el-icon><MoreFilled /></el-icon>
              </el-button>
              <template #dropdown>
                <el-dropdown-menu>
                  <el-dropdown-item @click="router.push('/profile')">
                    <el-icon><User /></el-icon> Profile
                  </el-dropdown-item>
                  <el-dropdown-item @click="toggleReceiveMessages">
                    <el-icon><Bell /></el-icon>
                    {{ authStore.currentUser?.permissions.canReceiveMessages ? 'Disable' : 'Enable' }} Notifications
                  </el-dropdown-item>
                  <el-dropdown-item divided @click="handleLogout">
                    <el-icon><SwitchButton /></el-icon> Logout
                  </el-dropdown-item>
                </el-dropdown-menu>
              </template>
            </el-dropdown>
          </div>
        </div>
      </div>

      <!-- Messages Area -->
      <div
        ref="messagesContainer"
        class="flex-1 overflow-y-auto p-4 space-y-4 bg-apple-light-gray dark:bg-apple-dark-gray"
      >
        <div
          v-for="message in chatStore.activeRoomMessages"
          :key="message.id"
          class="flex"
          :class="message.senderId === authStore.currentUser?.id ? 'justify-end' : 'justify-start'"
        >
          <div class="max-w-xs lg:max-w-md">
            <!-- Message Bubble -->
            <div
              class="message-bubble animate-slide-up"
              :class="message.senderId === authStore.currentUser?.id ? 'message-bubble-sent' : 'message-bubble-received'"
            >
              <!-- Sender Name (for received messages) -->
              <div
                v-if="message.senderId !== authStore.currentUser?.id"
                class="text-xs font-medium mb-1 opacity-70"
              >
                {{ message.senderName }}
              </div>

              <!-- Message Content -->
              <div class="break-words">
                {{ message.content }}
              </div>

              <!-- Message Actions -->
              <div class="flex items-center justify-between mt-1 text-xs opacity-70">
                <span>{{ formatTime(message.timestamp) }}</span>
                <div v-if="message.senderId === authStore.currentUser?.id" class="flex items-center space-x-1">
                  <span v-if="message.isEdited">edited</span>
                  <el-button
                    v-if="authStore.currentUser?.permissions.canDeleteOwnMessages"
                    size="small"
                    type="text"
                    @click="deleteMessage(message.id)"
                    class="!p-0 !h-auto text-white hover:text-red-200"
                  >
                    <el-icon><Delete /></el-icon>
                  </el-button>
                </div>
              </div>
            </div>
          </div>
        </div>

        <!-- Loading Indicator -->
        <div v-if="chatStore.isLoading" class="flex justify-center">
          <div class="loading-dots">
            <div></div>
            <div></div>
            <div></div>
          </div>
        </div>
      </div>

      <!-- Message Input -->
      <div class="p-4 bg-white dark:bg-gray-800 border-t border-gray-200 dark:border-gray-700">
        <div class="flex items-end space-x-3">
          <div class="flex-1">
            <el-input
              v-model="messageInput"
              type="textarea"
              :rows="1"
              :autosize="{ minRows: 1, maxRows: 4 }"
              placeholder="Type a message..."
              class="chat-input"
              @keydown.enter.exact.prevent="sendMessage"
              @keydown.enter.shift.exact="newLine"
              :disabled="!authStore.currentUser?.permissions.canSendMessages || !chatStore.isConnected"
            />
          </div>

          <el-button
            type="primary"
            circle
            size="large"
            @click="sendMessage"
            :disabled="!messageInput.trim() || !authStore.currentUser?.permissions.canSendMessages || !chatStore.isConnected"
            class="!bg-apple-blue !border-apple-blue hover:!bg-blue-600"
          >
            <el-icon><Promotion /></el-icon>
          </el-button>
        </div>

        <!-- Input Helper Text -->
        <div class="mt-2 text-xs text-gray-500 dark:text-gray-400">
          <span v-if="!authStore.currentUser?.permissions.canSendMessages">
            Messaging is disabled. Check your permissions.
          </span>
          <span v-else-if="!chatStore.isConnected">
            Disconnected. Trying to reconnect...
          </span>
          <span v-else>
            Press Enter to send, Shift+Enter for new line
          </span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, nextTick, watch } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  Sunny,
  Moon,
  Setting,
  MoreFilled,
  User,
  Bell,
  SwitchButton,
  Delete,
  Promotion
} from '@element-plus/icons-vue'
import { useDark, useToggle } from '@vueuse/core'
import { useAuthStore } from '@/stores/auth'
import { useChatStore } from '@/stores/chat'

const router = useRouter()
const authStore = useAuthStore()
const chatStore = useChatStore()

const isDarkMode = useDark()
const toggleDark = useToggle(isDarkMode)

const messageInput = ref('')
const messagesContainer = ref<HTMLElement>()

// Utility functions
const getUserColor = (username: string): string => {
  const colors = [
    '#007AFF', '#34C759', '#FF3B30', '#FF9500', '#FFCC00',
    '#AF52DE', '#FF2782', '#5AC8FA', '#5856D6', '#2AABEE'
  ]
  let hash = 0
  for (let i = 0; i < username.length; i++) {
    hash = username.charCodeAt(i) + ((hash << 5) - hash)
  }
  return colors[Math.abs(hash) % colors.length]
}

const getInitials = (name: string): string => {
  return name
    .split(' ')
    .map(word => word[0])
    .join('')
    .toUpperCase()
    .slice(0, 2)
}

const formatTime = (timestamp: Date): string => {
  const now = new Date()
  const diff = now.getTime() - timestamp.getTime()
  const seconds = Math.floor(diff / 1000)
  const minutes = Math.floor(seconds / 60)
  const hours = Math.floor(minutes / 60)
  const days = Math.floor(hours / 24)

  if (days > 0) {
    return timestamp.toLocaleDateString()
  } else if (hours > 0) {
    return `${hours}h ago`
  } else if (minutes > 0) {
    return `${minutes}m ago`
  } else {
    return 'Just now'
  }
}

// Actions
const sendMessage = async () => {
  if (!messageInput.value.trim()) return

  const success = await chatStore.sendMessage(messageInput.value.trim())
  if (success) {
    messageInput.value = ''
    scrollToBottom()
  } else {
    ElMessage.error('Failed to send message')
  }
}

const newLine = () => {
  messageInput.value += '\n'
}

const deleteMessage = async (messageId: string) => {
  try {
    await ElMessageBox.confirm(
      'Are you sure you want to delete this message?',
      'Delete Message',
      {
        confirmButtonText: 'Delete',
        cancelButtonText: 'Cancel',
        type: 'warning',
      }
    )

    const success = await chatStore.deleteOwnMessage(messageId)
    if (success) {
      ElMessage.success('Message deleted')
    } else {
      ElMessage.error('Failed to delete message')
    }
  } catch {
    // User canceled
  }
}

const toggleReceiveMessages = async () => {
  const newPermission = !authStore.currentUser?.permissions.canReceiveMessages
  const success = await authStore.updateUserPermissions({
    canReceiveMessages: newPermission
  })

  if (success) {
    ElMessage.success(
      newPermission ? 'Notifications enabled' : 'Notifications disabled'
    )
  } else {
    ElMessage.error('Failed to update permissions')
  }
}

const handleLogout = async () => {
  try {
    await ElMessageBox.confirm(
      'Are you sure you want to logout?',
      'Logout',
      {
        confirmButtonText: 'Logout',
        cancelButtonText: 'Cancel',
        type: 'warning',
      }
    )

    chatStore.disconnectWebSocket()
    authStore.logout()
    ElMessage.success('Logged out successfully')
    router.push('/login')
  } catch {
    // User canceled
  }
}

const scrollToBottom = () => {
  nextTick(() => {
    if (messagesContainer.value) {
      messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight
    }
  })
}

// Watchers
watch(() => chatStore.activeRoomMessages, () => {
  scrollToBottom()
}, { deep: true })

// Lifecycle
onMounted(async () => {
  await chatStore.loadRooms()
  chatStore.connectWebSocket()

  // Set default room if none selected
  if (!chatStore.activeRoomId && chatStore.rooms.length > 0) {
    chatStore.setActiveRoom(chatStore.rooms[0].id)
  }
})

onUnmounted(() => {
  chatStore.disconnectWebSocket()
})
</script>

<style scoped>
.gradient-text {
  background: linear-gradient(135deg, #007AFF 0%, #2AABEE 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.animate-slide-up {
  animation: slideUp 0.3s ease-out;
}

@keyframes slideUp {
  from {
    opacity: 0;
    transform: translateY(10px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

/* Scrollbar styles */
::-webkit-scrollbar {
  width: 6px;
}

::-webkit-scrollbar-track {
  background: transparent;
}

::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.2);
  border-radius: 3px;
}

::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.3);
}

.dark ::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
}

.dark ::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.3);
}
</style>

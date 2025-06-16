<template>
  <div class="flex" :class="isOwnMessage ? 'justify-end' : 'justify-start'">
    <div class="max-w-xs lg:max-w-md">
      <!-- Avatar for received messages -->
      <div
        v-if="!isOwnMessage && showAvatar"
        class="flex items-end space-x-2 mb-1"
      >
        <div
          class="user-avatar w-6 h-6 text-xs"
          :style="{ backgroundColor: getUserColor(message.senderName) }"
        >
          {{ getInitials(message.senderName) }}
        </div>
        <span class="text-xs text-gray-500 dark:text-gray-400">
          {{ message.senderName }}
        </span>
      </div>

      <!-- Message Bubble -->
      <div
        class="message-bubble animate-bounce-in"
        :class="[
          isOwnMessage ? 'message-bubble-sent' : 'message-bubble-received',
          { 'opacity-50': message.isDeleted }
        ]"
      >
        <!-- Message Content -->
        <div class="break-words" :class="{ 'italic': message.isDeleted }">
          {{ message.isDeleted ? 'This message was deleted' : message.content }}
        </div>

        <!-- Message Footer -->
        <div class="flex items-center justify-between mt-1 text-xs opacity-70">
          <span>{{ formatTime(message.timestamp) }}</span>
          <div v-if="isOwnMessage" class="flex items-center space-x-1">
            <span v-if="message.isEdited" class="text-xs">edited</span>
            <el-button
              v-if="canDelete && !message.isDeleted"
              size="small"
              type="text"
              @click="$emit('delete', message.id)"
              class="!p-0 !h-auto hover:text-red-300"
              :class="isOwnMessage ? 'text-white' : 'text-gray-500'"
            >
              <el-icon><Delete /></el-icon>
            </el-button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { Delete } from '@element-plus/icons-vue'
import type { Message } from '@/stores/chat'

interface Props {
  message: Message
  currentUserId?: string
  canDelete?: boolean
  showAvatar?: boolean
}

const props = withDefaults(defineProps<Props>(), {
  canDelete: false,
  showAvatar: true,
})

defineEmits<{
  delete: [messageId: string]
}>()

const isOwnMessage = computed(() => props.message.senderId === props.currentUserId)

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
</script>

<style scoped>
.animate-bounce-in {
  animation: bounceIn 0.3s ease-out;
}

@keyframes bounceIn {
  0% {
    opacity: 0;
    transform: scale(0.3);
  }
  50% {
    opacity: 1;
    transform: scale(1.05);
  }
  70% {
    transform: scale(0.9);
  }
  100% {
    opacity: 1;
    transform: scale(1);
  }
}
</style>

<template>
  <div class="chat-container">
    <!-- 侧边栏 -->
    <div class="sidebar">
      <div class="user-info">
        <div class="user-avatar">
          {{ user?.username?.charAt(0).toUpperCase() }}
        </div>
        <div class="user-details">
          <div class="username">{{ user?.username }}</div>
          <div class="status" :class="{ connected: isConnected }">
            {{ isConnected ? '在线' : '离线' }}
          </div>
        </div>
      </div>

      <div class="online-users">
        <h3>在线用户 ({{ onlineCount }})</h3>
        <div class="users-list">
          <div
            v-for="username in onlineUsers"
            :key="username"
            class="user-item"
          >
            <div class="user-avatar small">
              {{ username.charAt(0).toUpperCase() }}
            </div>
            <span>{{ username }}</span>
          </div>
        </div>
      </div>

      <div class="sidebar-actions">
        <button @click="showSettings = true" class="action-btn">
          <span>⚙️</span> 设置
        </button>
        <button @click="handleLogout" class="action-btn logout">
          <span>🚪</span> 退出
        </button>
      </div>
    </div>

    <!-- 主聊天区域 -->
    <div class="main-chat">
      <!-- 聊天消息区 -->
      <div class="messages-container" ref="messagesContainer">
        <div
          v-for="(message, index) in messages"
          :key="index"
          class="message-wrapper"
          :class="{
            'own-message': message.sender_username === user?.username,
            'system-message': message.type === 'system',
            'notification-message': message.type === 'notification'
          }"
        >
          <div v-if="message.type === 'chat'" class="chat-message">
            <div class="message-header">
              <span class="sender">{{ message.sender_username }}</span>
              <span class="timestamp">{{ formatTime(message.timestamp) }}</span>
              <button
                v-if="message.sender_username === user?.username && message.id"
                @click="deleteMessage(message.id)"
                class="delete-btn"
                title="撤回消息"
              >
                ❌
              </button>
            </div>
            <div class="message-content">
              {{ message.content }}
              <span v-if="message.filtered" class="filtered-indicator" title="消息已过滤">🔒</span>
            </div>
          </div>

          <div v-else class="system-notification">
            {{ message.content }}
          </div>
        </div>
      </div>

      <!-- 输入区域 -->
      <div class="input-area">
        <div class="input-container">
          <input
            v-model="newMessage"
            @keyup.enter="sendMessage"
            type="text"
            placeholder="输入消息..."
            :disabled="!isConnected"
            class="message-input"
          />
          <button
            @click="sendMessage"
            :disabled="!isConnected || !newMessage.trim()"
            class="send-btn"
          >
            发送
          </button>
        </div>
        <div v-if="!isConnected" class="connection-status">
          连接已断开，正在重连...
        </div>
      </div>
    </div>

    <!-- 设置弹窗 -->
    <div v-if="showSettings" class="modal-overlay" @click="showSettings = false">
      <div class="modal-content" @click.stop>
        <h3>用户设置</h3>
        <div class="setting-item">
          <label>
            <input
              v-model="acceptMessages"
              type="checkbox"
              @change="updateSettings"
            />
            接收消息
          </label>
        </div>
        <div class="modal-actions">
          <button @click="showSettings = false" class="btn-secondary">关闭</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted, nextTick, watch } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores/useUser'
import { useChatStore } from '@/stores/useChat'

const router = useRouter()
const authStore = useAuthStore()
const chatStore = useChatStore()

const newMessage = ref('')
const showSettings = ref(false)
const acceptMessages = ref(true)
const messagesContainer = ref<HTMLElement>()

// 计算属性
const user = computed(() => authStore.user)
const isConnected = computed(() => chatStore.isConnected)
const messages = computed(() => chatStore.messages)
const onlineUsers = computed(() => chatStore.onlineUsers)
const onlineCount = computed(() => chatStore.onlineCount)

// 初始化
onMounted(async () => {
  if (!authStore.isAuthenticated) {
    router.push('/login')
    return
  }

  // 初始化用户设置
  acceptMessages.value = user.value?.accept_messages ?? true

  // 初始化WebSocket连接
  if (authStore.token) {
    chatStore.initWebSocket(authStore.token)
  }
})

// 清理
onUnmounted(() => {
  chatStore.disconnect()
})

// 监听消息变化，自动滚动到底部
watch(messages, () => {
  nextTick(() => {
    scrollToBottom()
  })
}, { deep: true })

// 发送消息
const sendMessage = () => {
  if (!newMessage.value.trim() || !isConnected.value) {
    return
  }

  const success = chatStore.sendMessage(newMessage.value)
  if (success) {
    newMessage.value = ''
  }
}

// 删除消息
const deleteMessage = async (messageId: number) => {
  if (!confirm('确定要撤回这条消息吗？')) {
    return
  }

  const result = await chatStore.deleteMessage(messageId)
  if (!result.success) {
    alert(result.error || '撤回失败')
  }
}

// 更新设置
const updateSettings = async () => {
  const result = await authStore.updateSettings({
    accept_messages: acceptMessages.value
  })

  if (!result.success) {
    alert(result.error || '更新设置失败')
    acceptMessages.value = !acceptMessages.value // 回滚
  }
}

// 登出
const handleLogout = async () => {
  if (confirm('确定要退出聊天室吗？')) {
    chatStore.disconnect()
    await authStore.logout()
    router.push('/login')
  }
}

// 滚动到底部
const scrollToBottom = () => {
  if (messagesContainer.value) {
    messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight
  }
}

// 格式化时间
const formatTime = (timestamp: number) => {
  const date = new Date(timestamp)
  const now = new Date()

  if (date.toDateString() === now.toDateString()) {
    // 今天，显示时分
    return date.toLocaleTimeString('zh-CN', {
      hour: '2-digit',
      minute: '2-digit'
    })
  } else {
    // 其他日期，显示月日时分
    return date.toLocaleString('zh-CN', {
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit'
    })
  }
}
</script>

<style scoped>
.chat-container {
  display: flex;
  height: 100vh;
  background: #f5f5f5;
}

/* 侧边栏 */
.sidebar {
  width: 280px;
  background: white;
  border-right: 1px solid #e1e5e9;
  display: flex;
  flex-direction: column;
}

.user-info {
  padding: 20px;
  border-bottom: 1px solid #e1e5e9;
  display: flex;
  align-items: center;
  gap: 12px;
}

.user-avatar {
  width: 40px;
  height: 40px;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  font-weight: bold;
  font-size: 16px;
}

.user-avatar.small {
  width: 24px;
  height: 24px;
  font-size: 12px;
}

.user-details {
  flex: 1;
}

.username {
  font-weight: 600;
  color: #333;
}

.status {
  font-size: 12px;
  color: #999;
}

.status.connected {
  color: #27ae60;
}

.online-users {
  flex: 1;
  padding: 20px;
  overflow-y: auto;
}

.online-users h3 {
  margin: 0 0 15px 0;
  color: #555;
  font-size: 14px;
  font-weight: 600;
}

.users-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.user-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px;
  border-radius: 6px;
  font-size: 14px;
}

.sidebar-actions {
  padding: 20px;
  border-top: 1px solid #e1e5e9;
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.action-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px;
  border: none;
  border-radius: 6px;
  background: #f8f9fa;
  cursor: pointer;
  font-size: 14px;
  transition: background-color 0.2s;
}

.action-btn:hover {
  background: #e9ecef;
}

.action-btn.logout {
  color: #e74c3c;
}

/* 主聊天区域 */
.main-chat {
  flex: 1;
  display: flex;
  flex-direction: column;
}

.messages-container {
  flex: 1;
  padding: 20px;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.message-wrapper {
  display: flex;
  flex-direction: column;
}

.message-wrapper.own-message {
  align-items: flex-end;
}

.chat-message {
  max-width: 70%;
  background: white;
  border-radius: 12px;
  padding: 12px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
}

.own-message .chat-message {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
}

.message-header {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 4px;
  font-size: 12px;
  opacity: 0.7;
}

.sender {
  font-weight: 600;
}

.timestamp {
  margin-left: auto;
}

.delete-btn {
  background: none;
  border: none;
  cursor: pointer;
  font-size: 10px;
  opacity: 0.5;
  transition: opacity 0.2s;
}

.delete-btn:hover {
  opacity: 1;
}

.message-content {
  word-wrap: break-word;
  line-height: 1.4;
  position: relative;
}

.filtered-indicator {
  margin-left: 4px;
  font-size: 10px;
}

.system-notification {
  text-align: center;
  font-size: 12px;
  color: #666;
  font-style: italic;
  padding: 4px 8px;
  background: #f0f0f0;
  border-radius: 12px;
  align-self: center;
}

.notification-message .system-notification {
  background: #e3f2fd;
  color: #1976d2;
}

/* 输入区域 */
.input-area {
  padding: 20px;
  background: white;
  border-top: 1px solid #e1e5e9;
}

.input-container {
  display: flex;
  gap: 12px;
  align-items: center;
}

.message-input {
  flex: 1;
  padding: 12px;
  border: 2px solid #e1e5e9;
  border-radius: 24px;
  font-size: 14px;
  outline: none;
  transition: border-color 0.2s;
}

.message-input:focus {
  border-color: #667eea;
}

.message-input:disabled {
  background: #f5f5f5;
  cursor: not-allowed;
}

.send-btn {
  padding: 12px 24px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  border-radius: 24px;
  cursor: pointer;
  font-weight: 600;
  transition: opacity 0.2s;
}

.send-btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.connection-status {
  margin-top: 8px;
  font-size: 12px;
  color: #e74c3c;
  text-align: center;
}

/* 模态框 */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.modal-content {
  background: white;
  border-radius: 12px;
  padding: 24px;
  min-width: 300px;
  max-width: 90vw;
}

.modal-content h3 {
  margin: 0 0 20px 0;
  color: #333;
}

.setting-item {
  margin-bottom: 16px;
}

.setting-item label {
  display: flex;
  align-items: center;
  gap: 8px;
  cursor: pointer;
}

.modal-actions {
  display: flex;
  justify-content: flex-end;
  gap: 8px;
  margin-top: 20px;
}

.btn-secondary {
  padding: 8px 16px;
  border: 1px solid #ddd;
  background: white;
  border-radius: 6px;
  cursor: pointer;
}

.btn-secondary:hover {
  background: #f5f5f5;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .chat-container {
    flex-direction: column;
  }

  .sidebar {
    width: 100%;
    height: auto;
    max-height: 200px;
  }

  .main-chat {
    height: calc(100vh - 200px);
  }

  .chat-message {
    max-width: 85%;
  }
}
</style>

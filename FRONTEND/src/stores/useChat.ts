import { ref, computed } from 'vue'
import { defineStore } from 'pinia'
import { messageAPI, userAPI } from '@/services/api'
import { WebSocketService } from '@/services/websocket'
import type { Message, WebSocketMessage } from '@/types'

interface ChatMessage extends Message {
  type: 'chat' | 'system' | 'notification'
}

export const useChatStore = defineStore('chat', () => {
  const messages = ref<Message[]>([])
  const onlineUsers = ref<string[]>([])
  const onlineCount = ref(0)
  const ws = ref<WebSocketService | null>(null)
  const isConnected = ref(false)

  // 初始化WebSocket连接
  const initWebSocket = (token: string) => {
    if (ws.value) {
      ws.value.disconnect()
    }

    ws.value = new WebSocketService()

    ws.value.connect(token, {
      onConnect: () => {
        isConnected.value = true
        console.log('WebSocket connected')
      },

      onDisconnect: () => {
        isConnected.value = false
        console.log('WebSocket disconnected')
      },

      onMessage: (message: WebSocketMessage) => {
        if (message.type === 'chat_message' && message.sender_username && message.content && message.timestamp) {
          addMessage({
            id: message.id,
            sender_id: message.sender_id,
            sender_username: message.sender_username,
            content: message.content,
            timestamp: message.timestamp,
            type: 'chat',
            filtered: message.filtered
          })
        }
      },

      onUserJoined: (username: string, count: number) => {
        onlineCount.value = count
        addMessage({
          content: `${username} 加入聊天室`,
          timestamp: Date.now(),
          type: 'notification'
        })
        loadOnlineUsers()
      },

      onUserLeft: (username: string, count: number) => {
        onlineCount.value = count
        addMessage({
          content: `${username} 离开聊天室`,
          timestamp: Date.now(),
          type: 'notification'
        })
        loadOnlineUsers()
      },

      onMessageDeleted: (messageId: number, deletedBy: string) => {
        const messageIndex = messages.value.findIndex(m => m.id === messageId)
        if (messageIndex !== -1) {
          messages.value.splice(messageIndex, 1)
          addMessage({
            content: `${deletedBy} 撤回了一条消息`,
            timestamp: Date.now(),
            type: 'notification'
          })
        }
      },

      onSystemMessage: (message: string) => {
        addMessage({
          content: message,
          timestamp: Date.now(),
          type: 'system'
        })
      },

      onAuthSuccess: (username: string) => {
        console.log('WebSocket auth success:', username)
        loadHistoryMessages()
        loadOnlineUsers()
      },

      onAuthError: (error: string) => {
        console.error('WebSocket auth error:', error)
        addMessage({
          content: `认证失败: ${error}`,
          timestamp: Date.now(),
          type: 'system'
        })
      },

      onError: (error: string) => {
        console.error('WebSocket error:', error)
        addMessage({
          content: `连接错误: ${error}`,
          timestamp: Date.now(),
          type: 'system'
        })
      }
    })
  }

  // 发送消息
  const sendMessage = (content: string) => {
    if (ws.value?.isConnected() && content.trim()) {
      ws.value.sendMessage(content.trim())
      return true
    }
    return false
  }

  // 添加消息到列表
  const addMessage = (message: Message) => {
    messages.value.push(message)

    // 限制消息数量，避免内存过大
    if (messages.value.length > 500) {
      messages.value.splice(0, 100)
    }
  }

  // 加载历史消息
  const loadHistoryMessages = async () => {
    try {
      const response = await messageAPI.getMessages(50)
      const historyMessages = response.data.map((msg: any) => ({
        id: msg.id,
        sender_id: msg.sender_id,
        sender_username: msg.sender_username,
        content: msg.content,
        timestamp: msg.timestamp,
        type: 'chat' as const
      }))

      messages.value = [...historyMessages]
    } catch (error) {
      console.error('Failed to load history messages:', error)
      addMessage({
        content: '加载历史消息失败',
        timestamp: Date.now(),
        type: 'system'
      })
    }
  }

  // 删除消息
  const deleteMessage = async (messageId: number) => {
    try {
      await messageAPI.deleteMessage(messageId)
      return { success: true }
    } catch (error: any) {
      return {
        success: false,
        error: error.response?.data?.error || '删除消息失败'
      }
    }
  }

  // 加载在线用户
  const loadOnlineUsers = async () => {
    try {
      const response = await userAPI.getOnlineUsers()
      onlineUsers.value = response.data.users || []
      onlineCount.value = response.data.count || 0
    } catch (error) {
      console.error('Failed to load online users:', error)
    }
  }

  // 断开连接
  const disconnect = () => {
    if (ws.value) {
      ws.value.disconnect()
      ws.value = null
    }
    isConnected.value = false
    messages.value = []
    onlineUsers.value = []
    onlineCount.value = 0
  }

  // 清空消息
  const clearMessages = () => {
    messages.value = []
  }

  return {
    messages: computed(() => messages.value),
    onlineUsers: computed(() => onlineUsers.value),
    onlineCount: computed(() => onlineCount.value),
    isConnected: computed(() => isConnected.value),
    initWebSocket,
    sendMessage,
    deleteMessage,
    loadOnlineUsers,
    disconnect,
    clearMessages
  }
})

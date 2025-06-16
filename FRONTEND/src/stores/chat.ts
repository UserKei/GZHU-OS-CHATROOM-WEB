import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { useAuthStore } from './auth'

export interface Message {
  id: string
  senderId: string
  senderName: string
  content: string
  timestamp: Date
  isEdited: boolean
  isDeleted: boolean
  roomId?: string
  type: 'text' | 'image' | 'file' | 'system'
}

export interface ChatRoom {
  id: string
  name: string
  participants: string[]
  createdAt: Date
  lastMessage?: Message
}

export const useChatStore = defineStore('chat', () => {
  const authStore = useAuthStore()

  // State
  const messages = ref<Message[]>([])
  const rooms = ref<ChatRoom[]>([])
  const activeRoomId = ref<string | null>(null)
  const onlineUsers = ref<string[]>([])
  const isConnected = ref(false)
  const isLoading = ref(false)
  const error = ref<string | null>(null)

  // WebSocket connection
  const ws = ref<WebSocket | null>(null)

  // Getters
  const activeRoom = computed(() =>
    rooms.value.find(room => room.id === activeRoomId.value)
  )

  const activeRoomMessages = computed(() =>
    messages.value.filter(msg =>
      !activeRoomId.value || msg.roomId === activeRoomId.value
    ).sort((a, b) => a.timestamp.getTime() - b.timestamp.getTime())
  )

  const unreadCount = computed(() => {
    // TODO: Implement unread message counting logic
    return 0
  })

  // Actions
  const connectWebSocket = () => {
    if (!authStore.token) return

    try {
      // TODO: Replace with your WebSocket server URL
      ws.value = new WebSocket(`ws://localhost:8080/ws?token=${authStore.token}`)

      ws.value.onopen = () => {
        isConnected.value = true
        console.log('WebSocket connected')
      }

      ws.value.onmessage = (event) => {
        const data = JSON.parse(event.data)
        handleWebSocketMessage(data)
      }

      ws.value.onclose = () => {
        isConnected.value = false
        console.log('WebSocket disconnected')
        // Attempt to reconnect after 3 seconds
        setTimeout(connectWebSocket, 3000)
      }

      ws.value.onerror = (error) => {
        console.error('WebSocket error:', error)
        isConnected.value = false
      }
    } catch (err) {
      console.error('Failed to connect WebSocket:', err)
    }
  }

  const disconnectWebSocket = () => {
    if (ws.value) {
      ws.value.close()
      ws.value = null
      isConnected.value = false
    }
  }

  const handleWebSocketMessage = (data: any) => {
    switch (data.type) {
      case 'message':
        addMessage(data.message)
        break
      case 'user_joined':
        if (!onlineUsers.value.includes(data.userId)) {
          onlineUsers.value.push(data.userId)
        }
        break
      case 'user_left':
        onlineUsers.value = onlineUsers.value.filter(id => id !== data.userId)
        break
      case 'message_deleted':
        deleteMessage(data.messageId)
        break
      case 'users_online':
        onlineUsers.value = data.users
        break
    }
  }

  const sendMessage = async (content: string, type: 'text' | 'image' | 'file' = 'text') => {
    if (!authStore.currentUser || !isConnected.value) return false

    const message: Omit<Message, 'id'> = {
      senderId: authStore.currentUser.id,
      senderName: authStore.currentUser.username,
      content,
      timestamp: new Date(),
      isEdited: false,
      isDeleted: false,
      roomId: activeRoomId.value || undefined,
      type,
    }

    try {
      if (ws.value) {
        ws.value.send(JSON.stringify({
          type: 'send_message',
          message,
        }))
        return true
      }
      return false
    } catch (err) {
      error.value = 'Failed to send message'
      return false
    }
  }

  const addMessage = (message: Message) => {
    // Check for sensitive words (basic client-side filtering)
    // Note: Server should also filter sensitive words
    const filteredContent = filterSensitiveWords(message.content)

    const newMessage: Message = {
      ...message,
      content: filteredContent,
      timestamp: new Date(message.timestamp),
    }

    messages.value.push(newMessage)
  }

  const deleteMessage = (messageId: string) => {
    const messageIndex = messages.value.findIndex(msg => msg.id === messageId)
    if (messageIndex !== -1) {
      messages.value[messageIndex].isDeleted = true
      messages.value[messageIndex].content = 'This message was deleted'
    }
  }

  const deleteOwnMessage = async (messageId: string) => {
    if (!authStore.currentUser?.permissions.canDeleteOwnMessages) return false

    try {
      if (ws.value) {
        ws.value.send(JSON.stringify({
          type: 'delete_message',
          messageId,
        }))
        return true
      }
      return false
    } catch (err) {
      error.value = 'Failed to delete message'
      return false
    }
  }

  const loadChatHistory = async (roomId?: string) => {
    isLoading.value = true
    error.value = null

    try {
      // TODO: Replace with actual API call to your C++ backend
      const url = roomId ? `/api/messages?roomId=${roomId}` : '/api/messages'
      const response = await fetch(url, {
        headers: {
          'Authorization': `Bearer ${authStore.token}`,
        },
      })

      if (!response.ok) {
        throw new Error('Failed to load chat history')
      }

      const data = await response.json()
      messages.value = data.map((msg: any) => ({
        ...msg,
        timestamp: new Date(msg.timestamp),
      }))

      return true
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to load chat history'
      return false
    } finally {
      isLoading.value = false
    }
  }

  const setActiveRoom = (roomId: string | null) => {
    activeRoomId.value = roomId
    if (roomId) {
      loadChatHistory(roomId)
    }
  }

  const filterSensitiveWords = (content: string): string => {
    // Basic sensitive word filtering
    // Your C++ backend should have more comprehensive filtering
    const sensitiveWords = ['spam', 'abuse', 'inappropriate']
    let filtered = content

    sensitiveWords.forEach(word => {
      const regex = new RegExp(word, 'gi')
      filtered = filtered.replace(regex, '*'.repeat(word.length))
    })

    return filtered
  }

  const loadRooms = async () => {
    try {
      // TODO: Replace with actual API call to your C++ backend
      const response = await fetch('/api/rooms', {
        headers: {
          'Authorization': `Bearer ${authStore.token}`,
        },
      })

      if (!response.ok) {
        throw new Error('Failed to load rooms')
      }

      const data = await response.json()
      rooms.value = data.map((room: any) => ({
        ...room,
        createdAt: new Date(room.createdAt),
      }))

      return true
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to load rooms'
      return false
    }
  }

  return {
    // State
    messages,
    rooms,
    activeRoomId,
    onlineUsers,
    isConnected,
    isLoading,
    error,

    // Getters
    activeRoom,
    activeRoomMessages,
    unreadCount,

    // Actions
    connectWebSocket,
    disconnectWebSocket,
    sendMessage,
    deleteOwnMessage,
    loadChatHistory,
    setActiveRoom,
    loadRooms,
  }
})

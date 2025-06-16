interface ChatMessage {
  id?: number
  type: 'chat_message' | 'system' | 'user_joined' | 'user_left' | 'message_deleted' | 'auth_success' | 'auth_error' | 'error'
  sender_id?: number
  sender_username?: string
  username?: string
  content?: string
  message?: string
  timestamp?: number
  filtered?: boolean
  message_id?: number
  deleted_by?: string
  online_count?: number
}

interface WebSocketEventHandlers {
  onMessage?: (message: ChatMessage) => void
  onUserJoined?: (username: string, onlineCount: number) => void
  onUserLeft?: (username: string, onlineCount: number) => void
  onMessageDeleted?: (messageId: number, deletedBy: string) => void
  onSystemMessage?: (message: string) => void
  onAuthSuccess?: (username: string) => void
  onAuthError?: (error: string) => void
  onError?: (error: string) => void
  onConnect?: () => void
  onDisconnect?: () => void
}

class WebSocketService {
  private ws: WebSocket | null = null
  private token: string | null = null
  private handlers: WebSocketEventHandlers = {}
  private reconnectAttempts = 0
  private maxReconnectAttempts = 5
  private reconnectDelay = 1000
  private isManualClose = false

  constructor(private wsUrl: string = 'ws://localhost:8080/ws') {}

  connect(token: string, handlers: WebSocketEventHandlers = {}) {
    this.token = token
    this.handlers = handlers
    this.isManualClose = false

    try {
      this.ws = new WebSocket(this.wsUrl)
      this.setupEventListeners()
    } catch (error) {
      console.error('WebSocket connection failed:', error)
      this.handlers.onError?.('连接失败')
    }
  }

  private setupEventListeners() {
    if (!this.ws) return

    this.ws.onopen = () => {
      console.log('WebSocket connected')
      this.reconnectAttempts = 0

      // 发送认证消息
      if (this.token) {
        this.send({
          type: 'auth',
          token: this.token
        })
      }

      this.handlers.onConnect?.()
    }

    this.ws.onmessage = (event) => {
      try {
        const message: ChatMessage = JSON.parse(event.data)
        this.handleMessage(message)
      } catch (error) {
        console.error('Failed to parse WebSocket message:', error)
      }
    }

    this.ws.onclose = (event) => {
      console.log('WebSocket disconnected:', event.code, event.reason)
      this.handlers.onDisconnect?.()

      if (!this.isManualClose && this.reconnectAttempts < this.maxReconnectAttempts) {
        this.scheduleReconnect()
      }
    }

    this.ws.onerror = (error) => {
      console.error('WebSocket error:', error)
      this.handlers.onError?.('连接错误')
    }
  }

  private handleMessage(message: ChatMessage) {
    switch (message.type) {
      case 'chat_message':
        this.handlers.onMessage?.(message)
        break

      case 'user_joined':
        if (message.username && message.online_count !== undefined) {
          this.handlers.onUserJoined?.(message.username, message.online_count)
        }
        break

      case 'user_left':
        if (message.username && message.online_count !== undefined) {
          this.handlers.onUserLeft?.(message.username, message.online_count)
        }
        break

      case 'message_deleted':
        if (message.message_id && message.deleted_by) {
          this.handlers.onMessageDeleted?.(message.message_id, message.deleted_by)
        }
        break

      case 'system':
        if (message.message) {
          this.handlers.onSystemMessage?.(message.message)
        }
        break

      case 'auth_success':
        if (message.username) {
          this.handlers.onAuthSuccess?.(message.username)
        }
        break

      case 'auth_error':
        if (message.message) {
          this.handlers.onAuthError?.(message.message)
        }
        break

      case 'error':
        if (message.message) {
          this.handlers.onError?.(message.message)
        }
        break
    }
  }

  private scheduleReconnect() {
    setTimeout(() => {
      this.reconnectAttempts++
      console.log(`WebSocket reconnect attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts}`)

      if (this.token) {
        this.connect(this.token, this.handlers)
      }
    }, this.reconnectDelay * this.reconnectAttempts)
  }

  sendMessage(content: string) {
    if (this.ws?.readyState === WebSocket.OPEN && this.token) {
      this.send({
        type: 'chat_message',
        token: this.token,
        content
      })
    } else {
      this.handlers.onError?.('连接未建立或已断开')
    }
  }

  private send(data: any) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(data))
    }
  }

  disconnect() {
    this.isManualClose = true
    if (this.ws) {
      this.ws.close()
      this.ws = null
    }
  }

  isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN
  }
}

export { WebSocketService, type ChatMessage, type WebSocketEventHandlers }

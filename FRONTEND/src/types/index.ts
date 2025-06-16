// 用户相关类型
export interface User {
  id: number
  username: string
  email?: string
  accept_messages: boolean
}

export interface LoginRequest {
  username: string
  password: string
}

export interface RegisterRequest {
  username: string
  password: string
  email?: string
}

export interface AuthResponse {
  token: string
  user: User
}

// 消息相关类型
export interface Message {
  id?: number
  sender_id?: number
  sender_username?: string
  content: string
  timestamp: number
  filtered?: boolean
  type?: 'chat' | 'system' | 'notification'
}

export interface ChatMessage {
  type: 'chat_message'
  id?: number
  sender_id?: number
  sender_username?: string
  content: string
  timestamp: number
  filtered?: boolean
}

export interface SystemMessage {
  type: 'system' | 'user_joined' | 'user_left' | 'message_deleted' | 'auth_success' | 'auth_error' | 'error'
  message?: string
  username?: string
  timestamp: number
  online_count?: number
  message_id?: number
  deleted_by?: string
}

export type WebSocketMessage = ChatMessage | SystemMessage

// API响应类型
export interface ApiResponse<T = any> {
  data?: T
  message?: string
  error?: string
}

export interface OnlineUsersResponse {
  count: number
  users: string[]
}

// 用户设置
export interface UserSettings {
  accept_messages: boolean
}

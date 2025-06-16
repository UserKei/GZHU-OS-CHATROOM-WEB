import axios from 'axios'

const API_BASE_URL = 'http://localhost:8080/api'

// 创建axios实例
const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
})

// 请求拦截器 - 添加token
api.interceptors.request.use((config) => {
  const token = localStorage.getItem('token')
  if (token) {
    config.headers.Authorization = `Bearer ${token}`
  }
  return config
})

// 响应拦截器 - 处理认证错误
api.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error.response?.status === 401) {
      localStorage.removeItem('token')
      localStorage.removeItem('user')
      window.location.href = '/login'
    }
    return Promise.reject(error)
  }
)

// 用户相关API
export const authAPI = {
  register: (userData: { username: string; password: string; email?: string }) =>
    api.post('/register', userData),

  login: (credentials: { username: string; password: string }) =>
    api.post('/login', credentials),

  logout: () => api.post('/logout'),

  getUserSettings: () => api.get('/user/settings'),

  updateUserSettings: (settings: { accept_messages: boolean }) =>
    api.put('/user/settings', settings),
}

// 消息相关API
export const messageAPI = {
  getMessages: (limit = 50) => api.get(`/messages?limit=${limit}`),

  deleteMessage: (messageId: number) => api.delete(`/messages/${messageId}`),
}

// 用户相关API
export const userAPI = {
  getOnlineUsers: () => api.get('/users/online'),
}

// 健康检查
export const healthAPI = {
  check: () => api.get('/health'),
}

export default api

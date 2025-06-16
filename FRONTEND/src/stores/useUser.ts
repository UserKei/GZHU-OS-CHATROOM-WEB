import { ref, computed } from 'vue'
import { defineStore } from 'pinia'
import { authAPI } from '@/services/api'
import type { User } from '@/types'

export const useAuthStore = defineStore('auth', () => {
  const user = ref<User | null>(null)
  const token = ref<string | null>(localStorage.getItem('token'))
  const isAuthenticated = computed(() => !!token.value && !!user.value)

  // 初始化用户信息
  const initUser = () => {
    const storedUser = localStorage.getItem('user')
    if (storedUser) {
      try {
        user.value = JSON.parse(storedUser)
      } catch (error) {
        console.error('Failed to parse stored user:', error)
        clearAuth()
      }
    }
  }

  // 登录
  const login = async (credentials: { username: string; password: string }) => {
    try {
      const response = await authAPI.login(credentials)
      const { token: newToken, user: userData } = response.data

      token.value = newToken
      user.value = userData

      localStorage.setItem('token', newToken)
      localStorage.setItem('user', JSON.stringify(userData))

      return { success: true }
    } catch (error: any) {
      return {
        success: false,
        error: error.response?.data?.error || '登录失败'
      }
    }
  }

  // 注册
  const register = async (userData: { username: string; password: string; email?: string }) => {
    try {
      await authAPI.register(userData)
      return { success: true, message: '注册成功，请登录' }
    } catch (error: any) {
      return {
        success: false,
        error: error.response?.data?.error || '注册失败'
      }
    }
  }

  // 登出
  const logout = async () => {
    try {
      if (token.value) {
        await authAPI.logout()
      }
    } catch (error) {
      console.error('Logout API failed:', error)
    } finally {
      clearAuth()
    }
  }

  // 清除认证信息
  const clearAuth = () => {
    user.value = null
    token.value = null
    localStorage.removeItem('token')
    localStorage.removeItem('user')
  }

  // 更新用户设置
  const updateSettings = async (settings: { accept_messages: boolean }) => {
    try {
      await authAPI.updateUserSettings(settings)
      if (user.value) {
        user.value.accept_messages = settings.accept_messages
        localStorage.setItem('user', JSON.stringify(user.value))
      }
      return { success: true }
    } catch (error: any) {
      return {
        success: false,
        error: error.response?.data?.error || '更新设置失败'
      }
    }
  }

  // 初始化
  initUser()

  return {
    user: computed(() => user.value),
    token: computed(() => token.value),
    isAuthenticated,
    login,
    register,
    logout,
    updateSettings
  }
})

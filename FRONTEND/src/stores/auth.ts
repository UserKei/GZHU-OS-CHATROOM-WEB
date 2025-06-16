import { defineStore } from 'pinia'
import { ref, computed } from 'vue'

export interface User {
  id: string
  username: string
  email: string
  avatar?: string
  isOnline: boolean
  lastSeen?: Date
  permissions: {
    canSendMessages: boolean
    canDeleteOwnMessages: boolean
    canReceiveMessages: boolean
  }
}

export const useAuthStore = defineStore('auth', () => {
  // State
  const user = ref<User | null>(null)
  const token = ref<string | null>(localStorage.getItem('token'))
  const isLoading = ref(false)
  const error = ref<string | null>(null)

  // Getters
  const isAuthenticated = computed(() => !!token.value && !!user.value)
  const currentUser = computed(() => user.value)

  // Actions
  const login = async (username: string, password: string) => {
    isLoading.value = true
    error.value = null

    try {
      // TODO: Replace with actual API call to your C++ backend
      const response = await fetch('/api/auth/login', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ username, password }),
      })

      if (!response.ok) {
        throw new Error('Login failed')
      }

      const data = await response.json()

      token.value = data.token
      user.value = data.user

      localStorage.setItem('token', data.token)

      return true
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Login failed'
      return false
    } finally {
      isLoading.value = false
    }
  }

  const register = async (username: string, email: string, password: string) => {
    isLoading.value = true
    error.value = null

    try {
      // TODO: Replace with actual API call to your C++ backend
      const response = await fetch('/api/auth/register', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ username, email, password }),
      })

      if (!response.ok) {
        throw new Error('Registration failed')
      }

      const data = await response.json()

      token.value = data.token
      user.value = data.user

      localStorage.setItem('token', data.token)

      return true
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Registration failed'
      return false
    } finally {
      isLoading.value = false
    }
  }

  const logout = () => {
    token.value = null
    user.value = null
    localStorage.removeItem('token')
  }

  const updateUserPermissions = async (permissions: Partial<User['permissions']>) => {
    if (!user.value) return false

    try {
      // TODO: Replace with actual API call to your C++ backend
      const response = await fetch('/api/user/permissions', {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${token.value}`,
        },
        body: JSON.stringify(permissions),
      })

      if (!response.ok) {
        throw new Error('Failed to update permissions')
      }

      user.value.permissions = { ...user.value.permissions, ...permissions }
      return true
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to update permissions'
      return false
    }
  }

  const fetchUserProfile = async () => {
    if (!token.value) return false

    try {
      // TODO: Replace with actual API call to your C++ backend
      const response = await fetch('/api/user/profile', {
        headers: {
          'Authorization': `Bearer ${token.value}`,
        },
      })

      if (!response.ok) {
        throw new Error('Failed to fetch user profile')
      }

      const userData = await response.json()
      user.value = userData
      return true
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to fetch user profile'
      return false
    }
  }

  // Initialize on store creation
  if (token.value) {
    fetchUserProfile()
  }

  return {
    // State
    user,
    token,
    isLoading,
    error,

    // Getters
    isAuthenticated,
    currentUser,

    // Actions
    login,
    register,
    logout,
    updateUserPermissions,
    fetchUserProfile,
  }
})

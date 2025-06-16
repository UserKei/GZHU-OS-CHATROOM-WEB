<template>
  <div class="login-container">
    <div class="login-card">
      <h1 class="title">{{ isLogin ? '登录' : '注册' }} 聊天室</h1>

      <form @submit.prevent="handleSubmit" class="login-form">
        <div class="form-group">
          <label for="username">用户名</label>
          <input
            id="username"
            v-model="form.username"
            type="text"
            required
            placeholder="请输入用户名"
            :disabled="loading"
          />
        </div>

        <div v-if="!isLogin" class="form-group">
          <label for="email">邮箱 (可选)</label>
          <input
            id="email"
            v-model="form.email"
            type="email"
            placeholder="请输入邮箱地址"
            :disabled="loading"
          />
        </div>

        <div class="form-group">
          <label for="password">密码</label>
          <input
            id="password"
            v-model="form.password"
            type="password"
            required
            placeholder="请输入密码"
            :disabled="loading"
          />
        </div>

        <div v-if="!isLogin" class="form-group">
          <label for="confirmPassword">确认密码</label>
          <input
            id="confirmPassword"
            v-model="form.confirmPassword"
            type="password"
            required
            placeholder="请再次输入密码"
            :disabled="loading"
          />
        </div>

        <div v-if="error" class="error-message">
          {{ error }}
        </div>

        <div v-if="success" class="success-message">
          {{ success }}
        </div>

        <button type="submit" class="submit-btn" :disabled="loading">
          <span v-if="loading">处理中...</span>
          <span v-else>{{ isLogin ? '登录' : '注册' }}</span>
        </button>
      </form>

      <div class="switch-mode">
        <button @click="toggleMode" type="button" class="switch-btn">
          {{ isLogin ? '没有账号？点击注册' : '已有账号？点击登录' }}
        </button>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores/useUser'

const router = useRouter()
const authStore = useAuthStore()

const isLogin = ref(true)
const loading = ref(false)
const error = ref('')
const success = ref('')

const form = reactive({
  username: '',
  email: '',
  password: '',
  confirmPassword: ''
})

const toggleMode = () => {
  isLogin.value = !isLogin.value
  error.value = ''
  success.value = ''
  Object.assign(form, {
    username: '',
    email: '',
    password: '',
    confirmPassword: ''
  })
}

const validateForm = () => {
  if (!form.username.trim()) {
    error.value = '请输入用户名'
    return false
  }

  if (form.username.length < 3) {
    error.value = '用户名至少3个字符'
    return false
  }

  if (!form.password) {
    error.value = '请输入密码'
    return false
  }

  if (form.password.length < 6) {
    error.value = '密码至少6个字符'
    return false
  }

  if (!isLogin.value) {
    if (form.password !== form.confirmPassword) {
      error.value = '两次输入的密码不一致'
      return false
    }
  }

  return true
}

const handleSubmit = async () => {
  error.value = ''
  success.value = ''

  if (!validateForm()) {
    return
  }

  loading.value = true

  try {
    if (isLogin.value) {
      const result = await authStore.login({
        username: form.username,
        password: form.password
      })

      if (result.success) {
        success.value = '登录成功！'
        setTimeout(() => {
          router.push('/chat')
        }, 1000)
      } else {
        error.value = result.error || '登录失败'
      }
    } else {
      const result = await authStore.register({
        username: form.username,
        password: form.password,
        email: form.email || undefined
      })

      if (result.success) {
        success.value = result.message || '注册成功！'
        setTimeout(() => {
          isLogin.value = true
          Object.assign(form, {
            email: '',
            password: '',
            confirmPassword: ''
          })
        }, 1500)
      } else {
        error.value = result.error || '注册失败'
      }
    }
  } catch (err) {
    console.error('Form submission error:', err)
    error.value = '操作失败，请重试'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.login-container {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  padding: 20px;
}

.login-card {
  background: white;
  border-radius: 12px;
  padding: 40px;
  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
  width: 100%;
  max-width: 400px;
}

.title {
  text-align: center;
  color: #333;
  margin-bottom: 30px;
  font-size: 24px;
  font-weight: 600;
}

.login-form {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.form-group label {
  font-weight: 500;
  color: #555;
  font-size: 14px;
}

.form-group input {
  padding: 12px;
  border: 2px solid #e1e5e9;
  border-radius: 8px;
  font-size: 16px;
  transition: border-color 0.3s ease;
}

.form-group input:focus {
  outline: none;
  border-color: #667eea;
}

.form-group input:disabled {
  background-color: #f5f5f5;
  cursor: not-allowed;
}

.error-message {
  color: #e74c3c;
  background-color: #ffeaea;
  padding: 10px;
  border-radius: 6px;
  font-size: 14px;
  text-align: center;
}

.success-message {
  color: #27ae60;
  background-color: #eafaf1;
  padding: 10px;
  border-radius: 6px;
  font-size: 14px;
  text-align: center;
}

.submit-btn {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  padding: 14px;
  border-radius: 8px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  transition: opacity 0.3s ease;
}

.submit-btn:hover:not(:disabled) {
  opacity: 0.9;
}

.submit-btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.switch-mode {
  text-align: center;
  margin-top: 20px;
}

.switch-btn {
  background: none;
  border: none;
  color: #667eea;
  cursor: pointer;
  font-size: 14px;
  text-decoration: underline;
}

.switch-btn:hover {
  color: #764ba2;
}
</style>

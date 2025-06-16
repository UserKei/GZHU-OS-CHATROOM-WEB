<template>
  <div class="min-h-screen bg-gradient-to-br from-apple-blue via-telegram-blue to-purple-600 flex items-center justify-center p-4">
    <div class="w-full max-w-md">
      <!-- Logo and Title -->
      <div class="text-center mb-8">
        <div class="inline-flex items-center justify-center w-20 h-20 bg-white/20 backdrop-blur-xl rounded-3xl mb-6">
          <el-icon :size="32" class="text-white">
            <ChatDotRound />
          </el-icon>
        </div>
        <h1 class="text-3xl font-bold text-white mb-2">Welcome Back</h1>
        <p class="text-white/80">Sign in to continue chatting</p>
      </div>

      <!-- Login Form -->
      <div class="glass-effect rounded-3xl p-8 shadow-apple">
        <el-form
          ref="loginFormRef"
          :model="loginForm"
          :rules="loginRules"
          @submit.prevent="handleLogin"
          class="space-y-6"
        >
          <el-form-item prop="username" class="mb-6">
            <el-input
              v-model="loginForm.username"
              placeholder="Username"
              size="large"
              class="rounded-full"
              :prefix-icon="User"
            />
          </el-form-item>

          <el-form-item prop="password" class="mb-6">
            <el-input
              v-model="loginForm.password"
              type="password"
              placeholder="Password"
              size="large"
              class="rounded-full"
              :prefix-icon="Lock"
              show-password
            />
          </el-form-item>

          <el-button
            type="primary"
            size="large"
            class="w-full rounded-full py-3 gradient-bg border-none"
            :loading="authStore.isLoading"
            @click="handleLogin"
          >
            <span class="font-semibold">Sign In</span>
          </el-button>
        </el-form>

        <!-- Divider -->
        <div class="flex items-center my-6">
          <div class="flex-1 border-t border-gray-300 dark:border-gray-600"></div>
          <span class="px-4 text-sm text-gray-500 dark:text-gray-400">or</span>
          <div class="flex-1 border-t border-gray-300 dark:border-gray-600"></div>
        </div>

        <!-- Register Link -->
        <div class="text-center">
          <p class="text-gray-600 dark:text-gray-300">
            Don't have an account?
            <router-link
              to="/register"
              class="text-apple-blue hover:text-blue-600 font-medium ml-1"
            >
              Sign up
            </router-link>
          </p>
        </div>
      </div>

      <!-- Error Message -->
      <div
        v-if="authStore.error"
        class="mt-4 p-4 bg-red-100 dark:bg-red-900/20 border border-red-300 dark:border-red-700 rounded-2xl"
      >
        <p class="text-red-600 dark:text-red-400 text-sm text-center">
          {{ authStore.error }}
        </p>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage, type FormInstance, type FormRules } from 'element-plus'
import { ChatDotRound, User, Lock } from '@element-plus/icons-vue'
import { useAuthStore } from '@/stores/auth'

const router = useRouter()
const authStore = useAuthStore()

const loginFormRef = ref<FormInstance>()

const loginForm = reactive({
  username: '',
  password: ''
})

const loginRules: FormRules = {
  username: [
    { required: true, message: 'Please enter your username', trigger: 'blur' },
    { min: 3, max: 20, message: 'Username must be between 3 and 20 characters', trigger: 'blur' }
  ],
  password: [
    { required: true, message: 'Please enter your password', trigger: 'blur' },
    { min: 6, message: 'Password must be at least 6 characters', trigger: 'blur' }
  ]
}

const handleLogin = async () => {
  if (!loginFormRef.value) return

  await loginFormRef.value.validate(async (valid) => {
    if (valid) {
      const success = await authStore.login(loginForm.username, loginForm.password)

      if (success) {
        ElMessage.success('Welcome back!')
        router.push('/')
      } else {
        ElMessage.error(authStore.error || 'Login failed')
      }
    }
  })
}
</script>

<style scoped>
.glass-effect {
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  background: rgba(255, 255, 255, 0.9);
  border: 1px solid rgba(255, 255, 255, 0.2);
}

.dark .glass-effect {
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.gradient-bg {
  background: linear-gradient(135deg, #007AFF 0%, #2AABEE 100%);
}

.gradient-bg:hover {
  background: linear-gradient(135deg, #0056CC 0%, #229ED9 100%);
}
</style>

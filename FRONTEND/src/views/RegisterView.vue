<template>
  <div class="min-h-screen bg-gradient-to-br from-telegram-blue via-apple-blue to-indigo-600 flex items-center justify-center p-4">
    <div class="w-full max-w-md">
      <!-- Logo and Title -->
      <div class="text-center mb-8">
        <div class="inline-flex items-center justify-center w-20 h-20 bg-white/20 backdrop-blur-xl rounded-3xl mb-6">
          <el-icon :size="32" class="text-white">
            <UserFilled />
          </el-icon>
        </div>
        <h1 class="text-3xl font-bold text-white mb-2">Join the Chat</h1>
        <p class="text-white/80">Create your account to get started</p>
      </div>

      <!-- Register Form -->
      <div class="glass-effect rounded-3xl p-8 shadow-apple">
        <el-form
          ref="registerFormRef"
          :model="registerForm"
          :rules="registerRules"
          @submit.prevent="handleRegister"
          class="space-y-6"
        >
          <el-form-item prop="username" class="mb-6">
            <el-input
              v-model="registerForm.username"
              placeholder="Username"
              size="large"
              class="rounded-full"
              :prefix-icon="User"
            />
          </el-form-item>

          <el-form-item prop="email" class="mb-6">
            <el-input
              v-model="registerForm.email"
              type="email"
              placeholder="Email"
              size="large"
              class="rounded-full"
              :prefix-icon="Message"
            />
          </el-form-item>

          <el-form-item prop="password" class="mb-6">
            <el-input
              v-model="registerForm.password"
              type="password"
              placeholder="Password"
              size="large"
              class="rounded-full"
              :prefix-icon="Lock"
              show-password
            />
          </el-form-item>

          <el-form-item prop="confirmPassword" class="mb-6">
            <el-input
              v-model="registerForm.confirmPassword"
              type="password"
              placeholder="Confirm Password"
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
            @click="handleRegister"
          >
            <span class="font-semibold">Create Account</span>
          </el-button>
        </el-form>

        <!-- Divider -->
        <div class="flex items-center my-6">
          <div class="flex-1 border-t border-gray-300 dark:border-gray-600"></div>
          <span class="px-4 text-sm text-gray-500 dark:text-gray-400">or</span>
          <div class="flex-1 border-t border-gray-300 dark:border-gray-600"></div>
        </div>

        <!-- Login Link -->
        <div class="text-center">
          <p class="text-gray-600 dark:text-gray-300">
            Already have an account?
            <router-link
              to="/login"
              class="text-apple-blue hover:text-blue-600 font-medium ml-1"
            >
              Sign in
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
import { UserFilled, User, Message, Lock } from '@element-plus/icons-vue'
import { useAuthStore } from '@/stores/auth'

const router = useRouter()
const authStore = useAuthStore()

const registerFormRef = ref<FormInstance>()

const registerForm = reactive({
  username: '',
  email: '',
  password: '',
  confirmPassword: ''
})

const validateConfirmPassword = (rule: any, value: any, callback: any) => {
  if (value !== registerForm.password) {
    callback(new Error('Passwords do not match'))
  } else {
    callback()
  }
}

const registerRules: FormRules = {
  username: [
    { required: true, message: 'Please enter your username', trigger: 'blur' },
    { min: 3, max: 20, message: 'Username must be between 3 and 20 characters', trigger: 'blur' },
    { pattern: /^[a-zA-Z0-9_]+$/, message: 'Username can only contain letters, numbers, and underscores', trigger: 'blur' }
  ],
  email: [
    { required: true, message: 'Please enter your email', trigger: 'blur' },
    { type: 'email', message: 'Please enter a valid email address', trigger: 'blur' }
  ],
  password: [
    { required: true, message: 'Please enter your password', trigger: 'blur' },
    { min: 6, message: 'Password must be at least 6 characters', trigger: 'blur' }
  ],
  confirmPassword: [
    { required: true, message: 'Please confirm your password', trigger: 'blur' },
    { validator: validateConfirmPassword, trigger: 'blur' }
  ]
}

const handleRegister = async () => {
  if (!registerFormRef.value) return

  await registerFormRef.value.validate(async (valid) => {
    if (valid) {
      const success = await authStore.register(
        registerForm.username,
        registerForm.email,
        registerForm.password
      )

      if (success) {
        ElMessage.success('Account created successfully!')
        router.push('/')
      } else {
        ElMessage.error(authStore.error || 'Registration failed')
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
  background: linear-gradient(135deg, #2AABEE 0%, #007AFF 100%);
}

.gradient-bg:hover {
  background: linear-gradient(135deg, #229ED9 0%, #0056CC 100%);
}
</style>

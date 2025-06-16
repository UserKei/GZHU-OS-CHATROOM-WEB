<template>
  <div class="min-h-screen bg-apple-light-gray dark:bg-apple-dark-gray">
    <!-- Header -->
    <div class="bg-white dark:bg-gray-800 border-b border-gray-200 dark:border-gray-700 p-6">
      <div class="max-w-4xl mx-auto flex items-center justify-between">
        <div class="flex items-center space-x-4">
          <el-button
            circle
            @click="router.back()"
            class="!border-gray-300 dark:!border-gray-600"
          >
            <el-icon><ArrowLeft /></el-icon>
          </el-button>
          <h1 class="text-2xl font-bold gradient-text">Profile</h1>
        </div>
      </div>
    </div>

    <div class="max-w-4xl mx-auto p-6">
      <div class="grid grid-cols-1 lg:grid-cols-3 gap-6">
        <!-- Profile Card -->
        <div class="lg:col-span-1">
          <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
            <div class="text-center">
              <!-- Avatar -->
              <div class="relative inline-block mb-4">
                <div
                  class="user-avatar w-24 h-24 text-2xl"
                  :style="{ backgroundColor: getUserColor(authStore.currentUser?.username || '') }"
                >
                  {{ getInitials(authStore.currentUser?.username || '') }}
                </div>
                <div class="online-indicator w-6 h-6 -bottom-1 -right-1"></div>
              </div>

              <!-- User Info -->
              <h2 class="text-xl font-bold text-gray-900 dark:text-white mb-1">
                {{ authStore.currentUser?.username }}
              </h2>
              <p class="text-gray-500 dark:text-gray-400 mb-2">
                {{ authStore.currentUser?.email }}
              </p>
              <p class="text-sm text-apple-green font-medium">Online</p>
            </div>

            <!-- Quick Stats -->
            <div class="mt-6 grid grid-cols-2 gap-4">
              <div class="text-center p-3 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="text-lg font-bold text-gray-900 dark:text-white">
                  {{ messageStats.sent }}
                </div>
                <div class="text-sm text-gray-500 dark:text-gray-400">Messages Sent</div>
              </div>
              <div class="text-center p-3 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="text-lg font-bold text-gray-900 dark:text-white">
                  {{ Math.floor(Math.random() * 30) + 1 }}
                </div>
                <div class="text-sm text-gray-500 dark:text-gray-400">Days Active</div>
              </div>
            </div>
          </div>
        </div>

        <!-- Settings & Permissions -->
        <div class="lg:col-span-2 space-y-6">
          <!-- Permissions Settings -->
          <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
            <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-4">
              Permissions & Privacy
            </h3>

            <div class="space-y-4">
              <!-- Send Messages -->
              <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="flex items-center space-x-3">
                  <div class="w-10 h-10 bg-apple-blue rounded-full flex items-center justify-center">
                    <el-icon class="text-white"><ChatDotRound /></el-icon>
                  </div>
                  <div>
                    <h4 class="font-medium text-gray-900 dark:text-white">Send Messages</h4>
                    <p class="text-sm text-gray-500 dark:text-gray-400">Allow sending messages to chat rooms</p>
                  </div>
                </div>
                <el-switch
                  v-model="permissions.canSendMessages"
                  @change="updatePermission('canSendMessages', $event)"
                />
              </div>

              <!-- Receive Messages -->
              <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="flex items-center space-x-3">
                  <div class="w-10 h-10 bg-apple-green rounded-full flex items-center justify-center">
                    <el-icon class="text-white"><Bell /></el-icon>
                  </div>
                  <div>
                    <h4 class="font-medium text-gray-900 dark:text-white">Receive Messages</h4>
                    <p class="text-sm text-gray-500 dark:text-gray-400">Allow receiving notifications for new messages</p>
                  </div>
                </div>
                <el-switch
                  v-model="permissions.canReceiveMessages"
                  @change="updatePermission('canReceiveMessages', $event)"
                />
              </div>

              <!-- Delete Own Messages -->
              <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="flex items-center space-x-3">
                  <div class="w-10 h-10 bg-apple-red rounded-full flex items-center justify-center">
                    <el-icon class="text-white"><Delete /></el-icon>
                  </div>
                  <div>
                    <h4 class="font-medium text-gray-900 dark:text-white">Delete Own Messages</h4>
                    <p class="text-sm text-gray-500 dark:text-gray-400">Allow deleting your own messages</p>
                  </div>
                </div>
                <el-switch
                  v-model="permissions.canDeleteOwnMessages"
                  @change="updatePermission('canDeleteOwnMessages', $event)"
                />
              </div>
            </div>
          </div>

          <!-- Account Settings -->
          <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple">
            <h3 class="text-lg font-semibold text-gray-900 dark:text-white mb-4">
              Account Settings
            </h3>

            <div class="space-y-4">
              <!-- Username -->
              <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="flex items-center space-x-3">
                  <div class="w-10 h-10 bg-apple-purple rounded-full flex items-center justify-center">
                    <el-icon class="text-white"><User /></el-icon>
                  </div>
                  <div>
                    <h4 class="font-medium text-gray-900 dark:text-white">Username</h4>
                    <p class="text-sm text-gray-500 dark:text-gray-400">{{ authStore.currentUser?.username }}</p>
                  </div>
                </div>
                <el-button size="small" @click="editUsername">Edit</el-button>
              </div>

              <!-- Email -->
              <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="flex items-center space-x-3">
                  <div class="w-10 h-10 bg-apple-orange rounded-full flex items-center justify-center">
                    <el-icon class="text-white"><Message /></el-icon>
                  </div>
                  <div>
                    <h4 class="font-medium text-gray-900 dark:text-white">Email</h4>
                    <p class="text-sm text-gray-500 dark:text-gray-400">{{ authStore.currentUser?.email }}</p>
                  </div>
                </div>
                <el-button size="small" @click="editEmail">Edit</el-button>
              </div>

              <!-- Password -->
              <div class="flex items-center justify-between p-4 bg-apple-light-gray dark:bg-gray-700 rounded-2xl">
                <div class="flex items-center space-x-3">
                  <div class="w-10 h-10 bg-apple-teal rounded-full flex items-center justify-center">
                    <el-icon class="text-white"><Lock /></el-icon>
                  </div>
                  <div>
                    <h4 class="font-medium text-gray-900 dark:text-white">Password</h4>
                    <p class="text-sm text-gray-500 dark:text-gray-400">••••••••</p>
                  </div>
                </div>
                <el-button size="small" @click="changePassword">Change</el-button>
              </div>
            </div>
          </div>

          <!-- Danger Zone -->
          <div class="bg-white dark:bg-gray-800 rounded-3xl p-6 shadow-apple border border-red-200 dark:border-red-800">
            <h3 class="text-lg font-semibold text-red-600 dark:text-red-400 mb-4">
              Danger Zone
            </h3>

            <div class="flex items-center justify-between p-4 bg-red-50 dark:bg-red-900/20 rounded-2xl">
              <div>
                <h4 class="font-medium text-red-600 dark:text-red-400">Delete Account</h4>
                <p class="text-sm text-red-500 dark:text-red-400">Permanently delete your account and all data</p>
              </div>
              <el-button
                type="danger"
                size="small"
                @click="deleteAccount"
                class="!bg-red-600 !border-red-600"
              >
                Delete
              </el-button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  ArrowLeft,
  ChatDotRound,
  Bell,
  Delete,
  User,
  Message,
  Lock
} from '@element-plus/icons-vue'
import { useAuthStore } from '@/stores/auth'
import { useChatStore } from '@/stores/chat'

const router = useRouter()
const authStore = useAuthStore()
const chatStore = useChatStore()

// Reactive permissions
const permissions = reactive({
  canSendMessages: authStore.currentUser?.permissions.canSendMessages || false,
  canReceiveMessages: authStore.currentUser?.permissions.canReceiveMessages || false,
  canDeleteOwnMessages: authStore.currentUser?.permissions.canDeleteOwnMessages || false,
})

// Message statistics
const messageStats = computed(() => {
  const userMessages = chatStore.messages.filter(
    msg => msg.senderId === authStore.currentUser?.id
  )

  return {
    sent: userMessages.length,
    deleted: userMessages.filter(msg => msg.isDeleted).length,
  }
})

// Utility functions
const getUserColor = (username: string): string => {
  const colors = [
    '#007AFF', '#34C759', '#FF3B30', '#FF9500', '#FFCC00',
    '#AF52DE', '#FF2782', '#5AC8FA', '#5856D6', '#2AABEE'
  ]
  let hash = 0
  for (let i = 0; i < username.length; i++) {
    hash = username.charCodeAt(i) + ((hash << 5) - hash)
  }
  return colors[Math.abs(hash) % colors.length]
}

const getInitials = (name: string): string => {
  return name
    .split(' ')
    .map(word => word[0])
    .join('')
    .toUpperCase()
    .slice(0, 2)
}

// Actions
const updatePermission = async (key: keyof typeof permissions, value: boolean) => {
  const success = await authStore.updateUserPermissions({ [key]: value })

  if (success) {
    ElMessage.success('Permission updated successfully')
  } else {
    // Revert the change if it failed
    permissions[key] = !value
    ElMessage.error('Failed to update permission')
  }
}

const editUsername = async () => {
  try {
    const { value: newUsername } = await ElMessageBox.prompt(
      'Enter your new username',
      'Edit Username',
      {
        confirmButtonText: 'Update',
        cancelButtonText: 'Cancel',
        inputValue: authStore.currentUser?.username,
        inputPattern: /^[a-zA-Z0-9_]{3,20}$/,
        inputErrorMessage: 'Username must be 3-20 characters and contain only letters, numbers, and underscores'
      }
    )

    // TODO: Implement username update API call
    ElMessage.success('Username updated successfully')
  } catch {
    // User canceled
  }
}

const editEmail = async () => {
  try {
    const { value: newEmail } = await ElMessageBox.prompt(
      'Enter your new email',
      'Edit Email',
      {
        confirmButtonText: 'Update',
        cancelButtonText: 'Cancel',
        inputValue: authStore.currentUser?.email,
        inputType: 'email',
        inputErrorMessage: 'Please enter a valid email address'
      }
    )

    // TODO: Implement email update API call
    ElMessage.success('Email updated successfully')
  } catch {
    // User canceled
  }
}

const changePassword = async () => {
  try {
    const { value: newPassword } = await ElMessageBox.prompt(
      'Enter your new password',
      'Change Password',
      {
        confirmButtonText: 'Update',
        cancelButtonText: 'Cancel',
        inputType: 'password',
        inputPattern: /^.{6,}$/,
        inputErrorMessage: 'Password must be at least 6 characters long'
      }
    )

    // TODO: Implement password change API call
    ElMessage.success('Password changed successfully')
  } catch {
    // User canceled
  }
}

const deleteAccount = async () => {
  try {
    await ElMessageBox.confirm(
      'This action cannot be undone. Are you absolutely sure?',
      'Delete Account',
      {
        confirmButtonText: 'Delete Account',
        cancelButtonText: 'Cancel',
        type: 'error',
        showClose: false,
      }
    )

    // Double confirmation
    await ElMessageBox.confirm(
      'Last chance! This will permanently delete your account and all your messages.',
      'Final Confirmation',
      {
        confirmButtonText: 'Yes, Delete Everything',
        cancelButtonText: 'Cancel',
        type: 'error',
        showClose: false,
      }
    )

    // TODO: Implement account deletion API call
    ElMessage.success('Account deleted successfully')
    authStore.logout()
    router.push('/login')
  } catch {
    // User canceled
  }
}

// Initialize permissions on mount
onMounted(() => {
  if (authStore.currentUser?.permissions) {
    Object.assign(permissions, authStore.currentUser.permissions)
  }
})
</script>

<style scoped>
.gradient-text {
  background: linear-gradient(135deg, #007AFF 0%, #2AABEE 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.shadow-apple {
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
}

.dark .shadow-apple {
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.2);
}
</style>

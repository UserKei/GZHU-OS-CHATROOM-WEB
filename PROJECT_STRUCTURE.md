# 项目结构说明

## 后端结构 (BACKEND/)

```
BACKEND/
├── main.cpp                 # 主程序入口
├── CMakeLists.txt          # CMake构建配置
├── include/                # 头文件目录
│   ├── core/              # 核心模块头文件
│   │   ├── auth.h         # 认证管理
│   │   ├── database.h     # 数据库操作
│   │   └── websocket_manager.h # WebSocket管理
│   └── utils/             # 工具模块头文件
│       └── content_filter.h # 内容过滤
├── src/                   # 源文件目录
│   ├── core/              # 核心模块实现
│   │   ├── auth.cpp
│   │   ├── database.cpp
│   │   └── websocket_manager.cpp
│   └── utils/             # 工具模块实现
│       └── content_filter.cpp
├── build/                 # 构建目录 (gitignore)
└── .gitignore            # Git忽略文件
```

### 架构说明

- **main.cpp**: 服务器主程序，包含HTTP路由和WebSocket处理
- **core/**: 核心功能模块
  - `auth`: 用户认证、会话管理、密码加密
  - `database`: SQLite数据库操作，用户和消息管理
  - `websocket_manager`: WebSocket连接管理，实时消息广播
- **utils/**: 工具功能模块
  - `content_filter`: 敏感词过滤、内容安全检查

## 前端结构 (FRONTEND/)

```
FRONTEND/
├── index.html             # HTML入口
├── package.json           # 项目配置
├── vite.config.ts        # Vite配置
├── tsconfig.json         # TypeScript配置
├── src/
│   ├── main.ts           # 应用入口
│   ├── App.vue           # 根组件
│   ├── assets/           # 静态资源
│   │   └── main.css      # 全局样式
│   ├── components/       # 组件目录
│   │   ├── auth/         # 认证相关组件
│   │   └── chat/         # 聊天相关组件
│   ├── router/           # 路由配置
│   │   └── index.ts
│   ├── services/         # 服务层
│   │   ├── api.ts        # HTTP API调用
│   │   └── websocket.ts  # WebSocket服务
│   ├── stores/           # 状态管理
│   │   ├── useUser.ts    # 用户状态管理
│   │   └── useChat.ts    # 聊天状态管理
│   ├── types/            # TypeScript类型定义
│   │   └── index.ts
│   ├── utils/            # 工具函数
│   └── views/            # 页面组件
│       ├── LoginView.vue
│       └── ChatView.vue
└── .gitignore            # Git忽略文件
```

### 架构说明

- **services/**: 服务层，处理API调用和WebSocket通信
- **stores/**: 使用Pinia进行状态管理
  - `useUser`: 用户认证状态、个人信息管理
  - `useChat`: 聊天消息、在线用户、WebSocket连接管理
- **types/**: 统一的TypeScript类型定义，确保前后端数据契约
- **views/**: 页面级组件
- **components/**: 可复用组件，按功能模块分组

## 命名规范

### 前端命名规范

1. **文件命名**:
   - Vue组件: PascalCase (如 `ChatView.vue`)
   - TypeScript文件: camelCase (如 `api.ts`)
   - Pinia stores: useXxx格式 (如 `useUser.ts`, `useChat.ts`)

2. **目录命名**:
   - 小写 + 连字符 (如 `components/chat/`)

3. **变量命名**:
   - camelCase (如 `userName`, `isConnected`)
   - 常量: UPPER_SNAKE_CASE (如 `API_BASE_URL`)

### 后端命名规范

1. **文件命名**:
   - 头文件: snake_case.h (如 `content_filter.h`)
   - 源文件: snake_case.cpp (如 `websocket_manager.cpp`)

2. **类命名**:
   - PascalCase (如 `WebSocketManager`, `ContentFilter`)

3. **函数命名**:
   - snake_case (如 `get_user_by_id`, `filter_content`)

4. **变量命名**:
   - snake_case (如 `user_id`, `message_content`)

## 数据流

### 用户认证流程
1. 前端发送登录请求 → `authAPI.login()`
2. 后端验证用户信息 → 返回JWT Token
3. 前端存储Token → `useUser` store
4. 后续请求自动添加Authorization头

### 消息传输流程
1. 用户发送消息 → WebSocket
2. 后端接收 → 内容过滤 → 存储数据库
3. 广播给所有在线用户 → WebSocket
4. 前端接收 → 更新`useChat` store → 界面展示

### 实时通信
- 使用WebSocket建立双向通信
- 消息类型: chat_message, system, user_joined, user_left
- 自动重连机制保证连接稳定性

## 开发建议

1. **类型安全**: 充分利用TypeScript，定义完整的类型系统
2. **组件化**: 前端按功能模块划分组件，提高复用性
3. **状态管理**: 使用Pinia进行全局状态管理，避免Props层层传递
4. **错误处理**: 统一的错误处理机制，用户友好的错误提示
5. **性能优化**: 消息列表虚拟滚动、图片懒加载等
6. **安全性**: 输入验证、XSS防护、CSRF保护

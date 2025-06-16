# 多用户聊天室管理系统

基于Vue3 + C++ Crow框架的实时聊天系统，支持多用户同时在线聊天。

## 项目结构

```
kei/
├── BACKEND/          # C++后端服务器
│   ├── src/          # 源代码
│   ├── build/        # 构建目录 (被gitignore)
│   └── CMakeLists.txt
└── FRONTEND/         # Vue3前端
    ├── src/          # 源代码
    └── package.json
```

## 功能特性

### 核心功能
1. ✅ **多用户聊天** - 支持3个以上用户同时在线聊天
2. ✅ **用户管理** - 用户注册、登录、权限管理
3. ✅ **权限控制** - 用户可设置是否接收消息、撤销已发消息
4. ✅ **消息存储** - 聊天记录保存，支持历史消息查看
5. ✅ **敏感词过滤** - 自动过滤敏感内容
6. ✅ **实时通信** - WebSocket实现实时消息推送

### 技术特性
- **后端技术栈**: C++17 + Crow Framework + SQLite + OpenSSL
- **前端技术栈**: Vue3 + TypeScript + Vite + Pinia
- **通信协议**: HTTP REST API + WebSocket
- **数据存储**: SQLite数据库
- **认证系统**: Token-based认证
- **安全特性**: 密码哈希、会话管理、内容过滤

## 安装和运行

### 后端运行

1. **安装依赖** (macOS with Homebrew):
```bash
brew install crowcpp/crow/crow nlohmann-json sqlite3 openssl@3
```

2. **编译项目**:
```bash
cd BACKEND
mkdir build && cd build
cmake ..
make
```

3. **启动服务器**:
```bash
./chat_server
```
服务器将在 `http://localhost:8080` 启动

### 前端运行

1. **安装依赖**:
```bash
cd FRONTEND
npm install
```

2. **启动开发服务器**:
```bash
npm run dev
```
前端将在 `http://localhost:5173` 启动

## API 接口

### 用户相关
- `POST /api/register` - 用户注册
- `POST /api/login` - 用户登录
- `POST /api/logout` - 用户登出
- `GET /api/user/settings` - 获取用户设置
- `PUT /api/user/settings` - 更新用户设置

### 聊天相关
- `GET /api/messages` - 获取历史消息
- `DELETE /api/messages/:id` - 删除消息
- `GET /api/users/online` - 获取在线用户
- `WS /ws` - WebSocket连接

### 系统相关
- `GET /api/health` - 健康检查

## 数据库设计

### users 表
- `id` - 用户ID
- `username` - 用户名
- `password_hash` - 密码哈希
- `email` - 邮箱
- `is_active` - 是否激活
- `accept_messages` - 是否接收消息
- `created_at` - 创建时间
- `last_login` - 最后登录时间

### messages 表
- `id` - 消息ID
- `sender_id` - 发送者ID
- `content` - 消息内容（过滤后）
- `original_content` - 原始消息内容
- `is_deleted` - 是否已删除
- `created_at` - 创建时间
- `deleted_at` - 删除时间

### online_users 表
- `user_id` - 用户ID
- `last_heartbeat` - 最后心跳时间

## 使用说明

1. **注册账号**: 访问前端页面，点击注册创建新账号
2. **登录系统**: 使用用户名和密码登录
3. **发送消息**: 在聊天界面输入消息并发送
4. **查看历史**: 页面会自动加载历史聊天记录
5. **管理消息**: 点击自己的消息可以删除
6. **用户设置**: 可设置是否接收消息等权限

## 项目特色

### 系统架构
- 前后端分离设计
- RESTful API + WebSocket双重通信
- 多进程支持，高并发处理

### 安全性
- 密码SHA256加密存储
- JWT Token认证
- 敏感词自动过滤
- SQL注入防护

### 用户体验
- 现代化UI设计
- 实时消息推送
- 响应式布局
- 消息状态显示

### 数据管理
- 自动清理过期数据
- 消息持久化存储
- 用户在线状态管理
- 系统统计功能

## 开发说明

本项目为操作系统课程设计，主要展示：
- C语言多进程编程
- 进程间通信技术
- 文件处理和数据库操作
- 网络编程和WebSocket
- 现代Web前端开发

## 作者

UserKei - 操作系统课程设计项目

## 许可证

MIT License

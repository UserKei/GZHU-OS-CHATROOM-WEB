cmake_minimum_required(VERSION 3.15)
project(ChatRoomServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置brew前缀
if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE BREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CMAKE_PREFIX_PATH "${BREW_PREFIX}")
endif()

# 查找依赖
find_package(PkgConfig REQUIRED)

# 查找Crow (header-only library)
find_path(CROW_INCLUDE_DIR crow.h PATHS ${CMAKE_PREFIX_PATH}/include)

# 查找SQLite3
find_package(SQLite3 REQUIRED)

# 查找线程支持
find_package(Threads REQUIRED)

# 查找nlohmann/json
find_package(nlohmann_json REQUIRED)

# 包含目录
include_directories(${CMAKE_PREFIX_PATH}/include)
include_directories(include)

# 源文件
set(SOURCES
    src/main.cpp
    src/database/database_manager.cpp
    src/models/user.cpp
    src/models/message.cpp
    src/services/auth_service.cpp
    src/services/chat_service.cpp
    src/services/message_filter.cpp
    src/handlers/websocket_handler.cpp
    src/utils/json_utils.cpp
    src/utils/time_utils.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    SQLite::SQLite3
    Threads::Threads
    nlohmann_json::nlohmann_json
)

# 编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# 包含路径
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CROW_INCLUDE_DIR}
    ${CMAKE_PREFIX_PATH}/include
)

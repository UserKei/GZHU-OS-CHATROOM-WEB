cmake_minimum_required(VERSION 3.10)
project(ChatRoomServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# 尝试找到Crow
find_package(Crow QUIET)

if(NOT Crow_FOUND)
    # 如果找不到Crow，尝试使用pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(CROW QUIET IMPORTED_TARGET crow)
    endif()
    
    if(NOT CROW_FOUND)
        # Crow是header-only库，直接使用include目录
        find_path(CROW_INCLUDE_DIR crow.h PATHS
            /opt/homebrew/include
            /usr/local/include
            /usr/include
            PATH_SUFFIXES crow
        )
        
        if(NOT CROW_INCLUDE_DIR)
            message(FATAL_ERROR 
                "Crow not found. Please install Crow framework:\n"
                "  macOS: brew install crowcpp/crow/crow\n"
                "  Ubuntu: sudo apt-get install libcrow-dev\n"
                "  Or build from source: https://github.com/CrowCpp/Crow")
        endif()
    endif()
endif()

# Find other dependencies
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(chat_server
    main.cpp
    src/core/database.cpp
    src/core/auth.cpp
    src/utils/content_filter.cpp
    src/core/websocket_manager.cpp
)

# Include directories
target_include_directories(chat_server PRIVATE
    include
    /opt/homebrew/include
    ${SQLITE3_INCLUDE_DIRS}
)

if(CROW_INCLUDE_DIR)
    target_include_directories(chat_server PRIVATE ${CROW_INCLUDE_DIR})
endif()

# Link libraries
target_link_libraries(chat_server 
    ${SQLITE3_LIBRARIES}
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

if(Crow_FOUND)
    target_link_libraries(chat_server Crow::Crow)
elseif(CROW_FOUND)
    target_link_libraries(chat_server PkgConfig::CROW)
endif()

# Compiler flags
target_compile_options(chat_server PRIVATE ${SQLITE3_CFLAGS_OTHER})

# Enable debug info
set(CMAKE_BUILD_TYPE Debug)

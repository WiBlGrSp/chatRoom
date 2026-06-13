cmake_minimum_required(VERSION 3.8)

project(chatRoomClient)

# C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 搜索源文件
file(GLOB SRC_LIST
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 生成可执行文件
add_executable(${PROJECT_NAME}
    ${SRC_LIST}
)

# 头文件目录
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# pthread
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME}
    Threads::Threads
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
    -fsanitize=address
    -g
)

target_link_options(${PROJECT_NAME}
    PRIVATE
    -fsanitize=address
)
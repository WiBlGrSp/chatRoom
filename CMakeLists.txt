cmake_minimum_required(VERSION 3.8)

project(ChatRoom)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#输出clangd所需的compile_commands.json文件
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#设定输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib
)

# Debug配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -Wall
        -Wextra
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
    )

    add_link_options(
        -fsanitize=address
    )
endif()

#进入子目录
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)


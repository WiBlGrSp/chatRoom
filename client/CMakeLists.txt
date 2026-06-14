find_package(Threads REQUIRED)

file(GLOB CLIENT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(chatClient
    ${CLIENT_SRC}
)

target_include_directories(chatClient
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chatClient
    PRIVATE
    common
    Threads::Threads
)
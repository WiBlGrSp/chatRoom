find_package(Threads REQUIRED)

file(GLOB SERVER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(chatServer
    ${SERVER_SRC}
)

target_include_directories(chatServer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chatServer
    PRIVATE
    common
    Threads::Threads
)
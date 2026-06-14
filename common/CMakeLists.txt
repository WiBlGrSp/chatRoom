file(GLOB COMMON_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(common STATIC
    ${COMMON_SRC}
)

target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# Common library - shared types and utilities
add_library(z80common STATIC
    types.cpp
    utils.cpp
)

target_include_directories(z80common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

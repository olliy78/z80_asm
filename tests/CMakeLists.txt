# Test programs
# For now, simple test executables without framework

add_executable(test_lexer
    test_lexer.cpp
    ../src/assembler/lexer.cpp
)

target_link_libraries(test_lexer PRIVATE z80common)
target_include_directories(test_lexer PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Integration tests (later with Python)

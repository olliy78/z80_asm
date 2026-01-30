# Test programs
# For now, simple test executables without framework

add_executable(test_lexer
    test_lexer.cpp
    ../src/assembler/lexer.cpp
)

target_link_libraries(test_lexer PRIVATE z80common)
target_include_directories(test_lexer PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_instructions
    test_instructions.cpp
    ../src/assembler/z80_instructions.cpp
)

target_link_libraries(test_instructions PRIVATE z80common)
target_include_directories(test_instructions PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_parser
    test_parser.cpp
    ../src/assembler/parser.cpp
    ../src/assembler/lexer.cpp
    ../src/assembler/z80_instructions.cpp
    ../src/assembler/symbol_table.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_parser PRIVATE z80common)
target_include_directories(test_parser PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_parser_advanced
    test_parser_advanced.cpp
    ../src/assembler/parser.cpp
    ../src/assembler/lexer.cpp
    ../src/assembler/z80_instructions.cpp
    ../src/assembler/symbol_table.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_parser_advanced PRIVATE z80common)
target_include_directories(test_parser_advanced PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_expression
    test_expression.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/symbol_table.cpp
)

target_link_libraries(test_expression PRIVATE z80common)
target_include_directories(test_expression PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_db_dw
    test_db_dw.cpp
    ../src/assembler/parser.cpp
    ../src/assembler/lexer.cpp
    ../src/assembler/z80_instructions.cpp
    ../src/assembler/symbol_table.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_db_dw PRIVATE z80common)
target_include_directories(test_db_dw PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_bit_writer
    test_bit_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_bit_writer PRIVATE z80common)
target_include_directories(test_bit_writer PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_rel_writer
    test_rel_writer.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_rel_writer PRIVATE z80common)
target_include_directories(test_rel_writer PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_rel_output
    test_rel_output.cpp
    ../src/assembler/parser.cpp
    ../src/assembler/lexer.cpp
    ../src/assembler/z80_instructions.cpp
    ../src/assembler/symbol_table.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_rel_output PRIVATE z80common)
target_include_directories(test_rel_output PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_assembler_architecture
    test_assembler_architecture.cpp
    ../src/assembler/assembler.cpp
    ../src/assembler/output_writer.cpp
    ../src/assembler/parser.cpp
    ../src/assembler/lexer.cpp
    ../src/assembler/z80_instructions.cpp
    ../src/assembler/symbol_table.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_assembler_architecture PRIVATE z80common)
target_include_directories(test_assembler_architecture PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(test_real_program
    test_real_program.cpp
    ../src/assembler/assembler.cpp
    ../src/assembler/output_writer.cpp
    ../src/assembler/parser.cpp
    ../src/assembler/lexer.cpp
    ../src/assembler/z80_instructions.cpp
    ../src/assembler/symbol_table.cpp
    ../src/assembler/expression.cpp
    ../src/assembler/rel_writer.cpp
    ../src/assembler/bit_writer.cpp
)

target_link_libraries(test_real_program PRIVATE z80common)
target_include_directories(test_real_program PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Integration tests (later with Python)

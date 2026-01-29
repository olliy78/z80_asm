# Z80 Assembler (M80 compatible)
add_executable(m80
    main.cpp
    lexer.cpp
    parser.cpp
    preprocessor.cpp
    z80_instructions.cpp
    symbol_table.cpp
    expression.cpp
    listing.cpp
    rel_writer.cpp
    errors.cpp
)

target_link_libraries(m80 PRIVATE z80common)

# Install target
install(TARGETS m80 DESTINATION bin)

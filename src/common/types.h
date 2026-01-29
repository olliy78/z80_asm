/**
 * @file types.h
 * @brief Common type definitions for the Z80 assembler/linker
 * 
 * This file contains all fundamental types, enumerations, and structures
 * used throughout the Z80 assembler and linker implementation.
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace z80 {

/**
 * @defgroup BasicTypes Basic Type Definitions
 * @brief Fundamental types for assembly operations
 * @{
 */

/** @brief 8-bit unsigned byte type */
using Byte = uint8_t;

/** @brief 16-bit unsigned word type */
using Word = uint16_t;

/** @brief 16-bit address type for Z80 memory addressing */
using Address = uint16_t;

/** @} */

/**
 * @enum TokenType
 * @brief Token types recognized by the lexer
 * 
 * This enumeration defines all token types that can be recognized
 * during lexical analysis of Z80 assembly source code.
 */
enum class TokenType {
    // End of file/line
    EndOfFile,
    EndOfLine,
    
    // Identifiers and literals
    Identifier,
    Number,
    String,
    
    // Operators
    Plus,           // +
    Minus,          // -
    Multiply,       // *
    Divide,         // /
    Modulo,         // MOD
    And,            // AND
    Or,             // OR
    Xor,            // XOR
    Not,            // NOT
    ShiftLeft,      // SHL
    ShiftRight,     // SHR
    Equal,          // EQ
    NotEqual,       // NE
    LessThan,       // LT
    LessEqual,      // LE
    GreaterThan,    // GT
    GreaterEqual,   // GE
    
    // Delimiters
    Colon,          // :
    Comma,          // ,
    LeftParen,      // (
    RightParen,     // )
    LeftBracket,    // [
    RightBracket,   // ]
    Dollar,         // $ (location counter)
    
    // Directives (starting with .)
    Directive,
    
    // Mnemonics
    Mnemonic,
    
    // Registers
    Register,
    
    // Comment
    Comment,
};

/**
 * @struct Token
 * @brief Represents a single lexical token
 * 
 * Contains all information about a token including its type,
 * text representation, source location, and numeric value (for number tokens).
 */
struct Token {
    TokenType type;        ///< Type of the token
    std::string text;      ///< Text representation of the token
    int line;              ///< Line number in source file (1-based)
    int column;            ///< Column number in source file (1-based)
    int64_t numValue;      ///< Numeric value for number tokens
    
    Token() : type(TokenType::EndOfFile), line(0), column(0), numValue(0) {}
    Token(TokenType t, const std::string& txt, int ln, int col)
        : type(t), text(txt), line(ln), column(col), numValue(0) {}
};

/**
 * @enum SymbolType
 * @brief Types of symbols in the symbol table
 */
enum class SymbolType {
    Label,          // Address label
    Equ,            // EQU constant
    Aset,           // ASET variable
    Macro,          // MACRO definition
};

/**
 * @struct Symbol
 * @brief Represents a symbol table entry
 * 
 * Contains all information about a symbol including its name, type,
 * value, and various attributes like public/external visibility.
 */
struct Symbol {
    std::string name;      ///< Name of the symbol
    SymbolType type;       ///< Type of the symbol
    int64_t value;         ///< Value/address of the symbol
    bool defined;          ///< True if symbol has been defined
    bool isPublic;         ///< True if symbol is exported (PUBLIC)
    bool isExternal;       ///< True if symbol is imported (EXTERNAL)
    bool isRelocatable;    ///< True if symbol address is relocatable
    int definedLine;       ///< Line number where symbol was defined
    
    Symbol() : type(SymbolType::Label), value(0), defined(false),
               isPublic(false), isExternal(false), isRelocatable(false),
               definedLine(0) {}
};

/**
 * @enum SegmentType
 * @brief Types of program segments
 */
enum class SegmentType {
    Absolute,       // ASEG
    Code,           // CSEG
    Data,           // DSEG
};

/**
 * @enum RelocationType
 * @brief Relocation types for Microsoft .REL format
 */
enum class RelocationType {
    Absolute,       // No relocation needed
    ProgramRelative,// Relocate relative to program base
    DataRelative,   // Relocate relative to data base
    CommonRelative, // Relocate relative to common base
};

/**
 * @enum AddressingMode
 * @brief Z80 instruction addressing modes
 */
enum class AddressingMode {
    Implied,
    Immediate,
    ImmediateExtended,
    Register,
    RegisterIndirect,
    Direct,
    Indexed,
    Bit,
    Relative,
    Extended,
};

} // namespace z80

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace z80 {

// Basic types used throughout the project

using Byte = uint8_t;
using Word = uint16_t;
using Address = uint16_t;

// Token types for lexer
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

// Token structure
struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;
    int64_t numValue;  // For number tokens
    
    Token() : type(TokenType::EndOfFile), line(0), column(0), numValue(0) {}
    Token(TokenType t, const std::string& txt, int ln, int col)
        : type(t), text(txt), line(ln), column(col), numValue(0) {}
};

// Symbol types
enum class SymbolType {
    Label,          // Address label
    Equ,            // EQU constant
    Aset,           // ASET variable
    Macro,          // MACRO definition
};

// Symbol table entry
struct Symbol {
    std::string name;
    SymbolType type;
    int64_t value;
    bool defined;
    bool isPublic;
    bool isExternal;
    bool isRelocatable;
    int definedLine;
    
    Symbol() : type(SymbolType::Label), value(0), defined(false),
               isPublic(false), isExternal(false), isRelocatable(false),
               definedLine(0) {}
};

// Segment types
enum class SegmentType {
    Absolute,       // ASEG
    Code,           // CSEG
    Data,           // DSEG
};

// Relocation types for .REL format
enum class RelocationType {
    Absolute,       // No relocation needed
    ProgramRelative,// Relocate relative to program base
    DataRelative,   // Relocate relative to data base
    CommonRelative, // Relocate relative to common base
};

// Instruction addressing modes
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

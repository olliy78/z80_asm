#include "../src/assembler/lexer.h"
#include <iostream>
#include <cassert>

using namespace z80;

void testBasicTokens() {
    std::string source = "label: LD A,10h ; comment\n";
    Lexer lexer(source, "test.mac");
    
    Token t1 = lexer.nextToken();
    assert(t1.type == TokenType::Identifier);
    assert(t1.text == "label");
    
    Token t2 = lexer.nextToken();
    assert(t2.type == TokenType::Colon);
    
    std::cout << "testBasicTokens passed\n";
}

void testNumbers() {
    std::string source = "10 0x1A 1Fh 1010b 77o\n";
    Lexer lexer(source, "test.mac");
    
    Token t1 = lexer.nextToken();
    assert(t1.type == TokenType::Number);
    assert(t1.numValue == 10);
    
    Token t2 = lexer.nextToken();
    assert(t2.type == TokenType::Number);
    assert(t2.numValue == 0x1A);
    
    Token t3 = lexer.nextToken();
    assert(t3.type == TokenType::Number);
    assert(t3.numValue == 0x1F);
    
    Token t4 = lexer.nextToken();
    assert(t4.type == TokenType::Number);
    assert(t4.numValue == 0b1010);
    
    Token t5 = lexer.nextToken();
    assert(t5.type == TokenType::Number);
    assert(t5.numValue == 077);
    
    std::cout << "testNumbers passed\n";
}

void testDirectives() {
    std::string source = ".z80 .list .xlist\n";
    Lexer lexer(source, "test.mac");
    
    Token t1 = lexer.nextToken();
    assert(t1.type == TokenType::Directive);
    assert(t1.text == ".z80");
    
    Token t2 = lexer.nextToken();
    assert(t2.type == TokenType::Directive);
    assert(t2.text == ".list");
    
    std::cout << "testDirectives passed\n";
}

int main() {
    std::cout << "Running Lexer tests...\n";
    
    testBasicTokens();
    testNumbers();
    testDirectives();
    
    std::cout << "All tests passed!\n";
    return 0;
}

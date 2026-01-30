/**
 * @file main.cpp
 * @brief Main entry point for the M80-compatible Z80 assembler
 * 
 * This program assembles Z80 assembly language source files (.mac)
 * into Microsoft Relocatable Object format (.rel) with optional
 * listing output (.prn).
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include <iostream>
#include <string>
#include <fstream>
#include "parser.h"

int main(int argc, char* argv[]) {
    std::cout << "Z80 Assembler (M80 compatible) - Version 0.1.0\n";
    std::cout << "Copyright (c) 2026\n\n";
    
    if (argc < 2) {
        std::cerr << "Usage: m80 <source.mac> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  /L      - Generate listing (.prn)\n";
        std::cerr << "  /X      - Expand conditionals in listing\n";
        std::cerr << "  /Z      - Enable Z80 instruction set\n";
        return 1;
    }
    
    std::string inputFile = argv[1];
    
    // Check if file exists
    std::ifstream file(inputFile);
    if (!file.good()) {
        std::cerr << "Error: Cannot open file '" << inputFile << "'\n";
        return 1;
    }
    file.close();
    
    std::cout << "Assembling: " << inputFile << "\n";
    
    // Assemble the file
    z80::Parser parser;
    bool success = parser.assemble(inputFile);
    
    if (!success || parser.hasErrors()) {
        std::cerr << "\nAssembly failed with errors:\n";
        for (const auto& error : parser.getErrors()) {
            std::cerr << error.filename << ":" << error.line << ":" << error.column 
                      << ": " << error.message << "\n";
        }
        return 1;
    }
    
    // Generate output filename (.rel)
    std::string outputFile = inputFile;
    size_t dotPos = outputFile.find_last_of('.');
    if (dotPos != std::string::npos) {
        outputFile = outputFile.substr(0, dotPos);
    }
    outputFile += ".rel";
    
    // Write output
    std::cout << "Writing output: " << outputFile << "\n";
    if (!parser.writeREL(outputFile)) {
        std::cerr << "Error: Failed to write output file\n";
        return 1;
    }
    
    std::cout << "Assembly complete - " << parser.getLines().size() << " lines processed\n";
    std::cout << "Output written to: " << outputFile << "\n";    
    return 0;
}
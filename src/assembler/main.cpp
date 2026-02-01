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
#include <iomanip>
#include <algorithm>
#include <vector>
#include "parser.h"
#include "listing.h"

int main(int argc, char* argv[]) {
    std::cout << "Z80 Assembler (M80 compatible) - Version 0.1.0\n";
    std::cout << "Copyright (c) 2026\n\n";
    
    if (argc < 2) {
        std::cerr << "Usage: m80 <source.mac> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  /L      - Generate listing (.prn)\n";
        std::cerr << "  /S      - Include symbol table in listing\n";
        std::cerr << "  /X      - Expand conditionals in listing\n";
        std::cerr << "  /Z      - Enable Z80 instruction set\n";
        return 1;
    }
    
    std::string inputFile = argv[1];
    
    // Parse command-line options
    bool generateListing = false;
    bool includeSymbolTable = false;
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "/L" || arg == "/l") {
            generateListing = true;
        } else if (arg == "/S" || arg == "/s") {
            includeSymbolTable = true;
        }
        // Other options can be added here
    }
    
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
    
    // Show warnings (if any)
    bool hasWarnings = false;
    bool hasErrors = false;
    for (const auto& error : parser.getErrors()) {
        if (error.level == z80::ErrorLevel::Warning) {
            hasWarnings = true;
        } else if (error.level == z80::ErrorLevel::Error) {
            hasErrors = true;
        }
    }
    
    if (hasWarnings) {
        std::cerr << "\nWarnings:\n";
        for (const auto& error : parser.getErrors()) {
            if (error.level == z80::ErrorLevel::Warning) {
                std::cerr << error.filename << ":" << error.line << ":" << error.column 
                          << ": " << error.message << "\n";
            }
        }
    }
    
    if (!success || hasErrors) {
        std::cerr << "\nAssembly failed with errors:\n";
        for (const auto& error : parser.getErrors()) {
            if (error.level == z80::ErrorLevel::Error) {
                std::cerr << error.filename << ":" << error.line << ":" << error.column 
                          << ": " << error.message << "\n";
            }
        }
        
        // Generate listing even on error for debugging (if requested)
        if (generateListing) {
            std::string listingFile = inputFile;
            size_t listDotPos = listingFile.find_last_of('.');
            if (listDotPos != std::string::npos) {
                listingFile = listingFile.substr(0, listDotPos);
            }
            listingFile += ".prn";
            
            std::cerr << "\nGenerating listing for error analysis: " << listingFile << "\n";
            
            z80::Listing listing(listingFile);
            
            // Add all assembled lines to listing (even partial)
            for (const auto& line : parser.getLines()) {
                z80::ListingLine listLine;
                listLine.lineNumber = line.lineNumber;
                listLine.address = line.address;
                listLine.code = line.code;
                
                // Reconstruct source line from parsed components
                std::string sourceLine;
                
                // Check if line is just a directive (no code, no label ending with :)
                bool isDirective = line.code.empty() && !line.mnemonic.empty() &&
                                  (line.mnemonic == "NAME" || line.mnemonic == "TITLE" ||
                                   line.mnemonic == "ORG" || line.mnemonic == "END" ||
                                   line.mnemonic == "PUBLIC" || line.mnemonic == "EXTRN" ||
                                   line.mnemonic == "ASEG" || line.mnemonic == "CSEG" || line.mnemonic == "DSEG");
                
                if (!line.label.empty() && !isDirective) {
                    sourceLine = line.label;
                    // Only add colon if it's a real label (not a directive name)
                    if (line.label.find(':') == std::string::npos) {
                        sourceLine += ":";
                    }
                    if (!line.mnemonic.empty()) sourceLine += " ";
                }
                if (!line.mnemonic.empty()) {
                    if (isDirective && line.label.empty()) {
                        sourceLine = "    ";  // Indent directives
                    }
                    sourceLine += line.mnemonic;
                    if (!line.operandString.empty()) {
                        sourceLine += " " + line.operandString;
                    }
                }
                if (!line.comment.empty()) {
                    if (!sourceLine.empty()) sourceLine += " ";
                    sourceLine += "; " + line.comment;
                }
                
                listLine.source = sourceLine;
                listLine.hasAddress = !line.code.empty();
                listLine.isError = false;
                
                listing.addLine(listLine);
            }
            
            if (!listing.close()) {
                std::cerr << "Warning: Failed to write listing file\n";
            }
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
    
    // Generate listing if requested
    if (generateListing) {
        std::string listingFile = inputFile;
        size_t listDotPos = listingFile.find_last_of('.');
        if (listDotPos != std::string::npos) {
            listingFile = listingFile.substr(0, listDotPos);
        }
        listingFile += ".prn";
        
        std::cout << "Generating listing: " << listingFile << "\n";
        
        z80::Listing listing(listingFile);
        
        // Add all assembled lines to listing
        for (const auto& line : parser.getLines()) {
            z80::ListingLine listLine;
            listLine.lineNumber = line.lineNumber;
            listLine.address = line.address;
            listLine.code = line.code;
            
            // Reconstruct source line from parsed components
            std::string sourceLine;
            
            // Check if line is just a directive (no code, no label ending with :)
            bool isDirective = line.code.empty() && !line.mnemonic.empty() &&
                              (line.mnemonic == "NAME" || line.mnemonic == "TITLE" ||
                               line.mnemonic == "ORG" || line.mnemonic == "END" ||
                               line.mnemonic == "PUBLIC" || line.mnemonic == "EXTRN" ||
                               line.mnemonic == "ASEG" || line.mnemonic == "CSEG" || line.mnemonic == "DSEG");
            
            if (!line.label.empty() && !isDirective) {
                sourceLine = line.label;
                // Only add colon if it's a real label (not a directive name)
                if (line.label.find(':') == std::string::npos) {
                    sourceLine += ":";
                }
                if (!line.mnemonic.empty()) sourceLine += " ";
            }
            if (!line.mnemonic.empty()) {
                if (isDirective && line.label.empty()) {
                    sourceLine = "    ";  // Indent directives
                }
                sourceLine += line.mnemonic;
                if (!line.operandString.empty()) {
                    sourceLine += " " + line.operandString;
                }
            }
            if (!line.comment.empty()) {
                if (!sourceLine.empty()) sourceLine += " ";
                sourceLine += "; " + line.comment;
            }
            
            listLine.source = sourceLine;
            listLine.hasAddress = !line.code.empty();
            listLine.isError = false;
            
            listing.addLine(listLine);
        }
        
        // Close the listing file first
        if (!listing.close()) {
            std::cerr << "Warning: Failed to write listing file\n";
            return 1;
        }
        
        // Append symbol table if requested
        if (includeSymbolTable) {
            std::ofstream symFile(listingFile, std::ios::app);
            if (symFile.is_open()) {
                // Get all symbols sorted alphabetically
                auto allSymbols = parser.getSymbolTable().getAllSymbols();
                std::vector<std::pair<std::string, z80::Symbol>> sortedSymbols(allSymbols.begin(), allSymbols.end());
                std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                          [](const auto& a, const auto& b) { return a.first < b.first; });
                
                // Symbol table header with form feed
                symFile << "\f\n";
                // Extract filename for page header
                std::string headerName = listingFile;
                size_t dotPos = headerName.find_last_of('.');
                if (dotPos != std::string::npos) {
                    headerName = headerName.substr(0, dotPos);
                }
                size_t slashPos = headerName.find_last_of("/\\");
                if (slashPos != std::string::npos) {
                    headerName = headerName.substr(slashPos + 1);
                }
                symFile << std::left << std::setw(60) << headerName << "PAGE   2\n\n";
                symFile << "Symbol Table:\n\n";
                
                // Write symbols (2 columns, better formatted)
                const int columnsPerLine = 2;
                int column = 0;
                
                for (const auto& pair : sortedSymbols) {
                    const std::string& name = pair.first;
                    const auto& sym = pair.second;
                    
                    // Format: NAME          VVVV T
                    symFile << std::left << std::setw(14) << name << " ";
                    symFile << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << sym.value;
                    symFile << std::setfill(' ');
                    
                    // Type indicator
                    if (sym.segment == z80::SegmentType::CSEG) symFile << " C";
                    else if (sym.segment == z80::SegmentType::DSEG) symFile << " D";
                    else if (sym.segment == z80::SegmentType::ASEG) symFile << " A";
                    else symFile << "  ";
                    
                    if (sym.isPublic) symFile << " Pub";
                    if (sym.isExternal) symFile << " Ext";
                    
                    symFile << "   ";
                    
                    column++;
                    if (column >= columnsPerLine) {
                        symFile << "\n";
                        column = 0;
                    }
                }
                if (column != 0) symFile << "\n";
                
                symFile.close();
            }
        }
        
        std::cout << "Listing written to: " << listingFile << "\n";
    }
    
    return 0;
}
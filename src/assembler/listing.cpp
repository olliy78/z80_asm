/**
 * @file listing.cpp
 * @brief Implementation of listing file generator
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#include "listing.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace z80 {

Listing::Listing(const std::string& filename, const std::string& title)
    : filename_(filename)
    , title_(title)
    , pageLength_(50)
    , currentLine_(0)
    , pageNumber_(0)
    , symbolTableOutput_(true)
{
    file_.open(filename);
    if (!file_.is_open()) {
        // Error handling
        return;
    }
    
    // If title is empty, derive from filename
    if (title_.empty()) {
        size_t lastSlash = filename.find_last_of("/\\");
        size_t lastDot = filename.find_last_of('.');
        size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
        size_t end = (lastDot == std::string::npos) ? filename.length() : lastDot;
        title_ = filename.substr(start, end - start);
    }
}

Listing::~Listing() {
    if (file_.is_open()) {
        close();
    }
}

void Listing::setTitle(const std::string& title) {
    title_ = title;
}

void Listing::setPageLength(int lines) {
    if (lines >= 10 && lines <= 255) {
        pageLength_ = lines;
    }
}

void Listing::setSymbolTableOutput(bool enable) {
    symbolTableOutput_ = enable;
}

void Listing::addLine(const ListingLine& line) {
    lines_.push_back(line);
}

void Listing::writePageHeader() {
    // M80 format: Title line, blank line, then listing
    // Page header format:
    // TITLE                                                        PAGE nnn
    
    pageNumber_++;
    currentLine_ = 0;
    
    // Write title and page number
    file_ << std::left << std::setw(60) << title_ 
          << "PAGE " << std::setw(3) << std::setfill('0') << pageNumber_ << "\n";
    file_ << std::setfill(' ');
    file_ << "\n";
    currentLine_ += 2;
}

void Listing::checkPageBreak() {
    if (currentLine_ >= pageLength_) {
        file_ << "\f";  // Form feed
        writePageHeader();
    }
}

std::string Listing::formatAddress(Address addr) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << addr;
    return oss.str();
}

std::string Listing::formatMachineCode(const std::vector<Byte>& code, size_t offset, size_t count) {
    std::ostringstream oss;
    for (size_t i = 0; i < count && (offset + i) < code.size(); ++i) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
            << static_cast<int>(code[offset + i]);
    }
    return oss.str();
}

void Listing::writeListingLine(const ListingLine& line) {
    checkPageBreak();
    
    // M80 format:
    // AAAA BBBBBBBBBB   Source
    // Where AAAA = address (4 hex), BBBBBBBBBB = up to 5 bytes of code (10 hex digits)
    
    std::ostringstream oss;
    
    if (line.hasAddress) {
        // Write address
        oss << formatAddress(line.address) << " ";
        
        // Write first 5 bytes of machine code
        if (!line.code.empty()) {
            size_t bytesToWrite = std::min<size_t>(5, line.code.size());
            std::string codeStr = formatMachineCode(line.code, 0, bytesToWrite);
            oss << std::left << std::setw(10) << codeStr << " ";
        } else {
            oss << std::left << std::setw(10) << "" << " ";
        }
    } else {
        // No address - just spacing
        oss << "     " << std::left << std::setw(10) << "" << " ";
    }
    
    // Write source line
    oss << line.source;
    
    // Write error if present
    if (line.isError && !line.errorMessage.empty()) {
        oss << "  ; ERROR: " << line.errorMessage;
    }
    
    file_ << oss.str() << "\n";
    currentLine_++;
    
    // If more than 5 bytes of code, write continuation lines
    if (line.code.size() > 5) {
        for (size_t offset = 5; offset < line.code.size(); offset += 5) {
            checkPageBreak();
            size_t bytesToWrite = std::min<size_t>(5, line.code.size() - offset);
            std::string codeStr = formatMachineCode(line.code, offset, bytesToWrite);
            file_ << "     " << std::left << std::setw(10) << codeStr << "\n";
            currentLine_++;
        }
    }
}

void Listing::writeSymbolTable(const SymbolTable& symbolTable) {
    // Get all symbols sorted alphabetically
    auto allSymbols = symbolTable.getAllSymbols();
    std::vector<std::pair<std::string, Symbol>> sortedSymbols(allSymbols.begin(), allSymbols.end());
    std::sort(sortedSymbols.begin(), sortedSymbols.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    
    // Page break before symbol table
    file_ << "\f";
    writePageHeader();
    
    // Symbol table header
    file_ << "\nSymbol Table:\n\n";
    currentLine_ += 3;
    
    // Write symbols in columns (3 per line)
    // Format: NAME  AAAA  TYPE
    const int columnsPerLine = 3;
    int column = 0;
    
    for (const auto& pair : sortedSymbols) {
        const std::string& name = pair.first;
        const Symbol& sym = pair.second;
        
        checkPageBreak();
        
        // Format: NAME (8 chars) VALUE (4 hex) TYPE
        std::ostringstream oss;
        oss << std::left << std::setw(10) << name.substr(0, 8) << " ";
        
        if (sym.defined) {
            oss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << sym.value;
        } else {
            oss << "????";
        }
        
        // Type indicators
        std::string typeStr;
        if (sym.isPublic) typeStr += " Pub";
        if (sym.isExternal) typeStr += " Ext";
        if (sym.segment == SegmentType::CSEG) typeStr += " C";
        else if (sym.segment == SegmentType::DSEG) typeStr += " D";
        else if (sym.segment == SegmentType::ASEG) typeStr += " A";
        if (sym.type == SymbolType::Equ) typeStr += " Equ";
        
        oss << std::left << std::setw(20) << typeStr;
        
        file_ << oss.str();
        column++;
        
        if (column >= columnsPerLine) {
            file_ << "\n";
            currentLine_++;
            column = 0;
        } else {
            file_ << "  ";
        }
    }
    
    if (column != 0) {
        file_ << "\n";
        currentLine_++;
    }
}

bool Listing::close() {
    if (!file_.is_open()) {
        return false;
    }
    
    // Write first page header if not yet written
    if (pageNumber_ == 0) {
        writePageHeader();
    }
    
    // Write all accumulated lines (only if not already written)
    if (!lines_.empty()) {
        for (const auto& line : lines_) {
            writeListingLine(line);
        }
        lines_.clear();  // Clear after writing
    }
    
    file_.close();
    return true;
}

} // namespace z80

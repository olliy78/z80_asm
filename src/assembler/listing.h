#pragma once

#include <string>

namespace z80 {

class Listing {
public:
    Listing(const std::string& filename);
    
    // TODO: Generate .PRN listing file
    void writeLine(const std::string& line);
    void close();
    
private:
    std::string filename_;
};

} // namespace z80

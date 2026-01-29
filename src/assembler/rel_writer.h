#pragma once

#include <string>

namespace z80 {

class RelWriter {
public:
    RelWriter(const std::string& filename);
    
    // TODO: Write Microsoft .REL format
    void write();
    void close();
    
private:
    std::string filename_;
};

} // namespace z80

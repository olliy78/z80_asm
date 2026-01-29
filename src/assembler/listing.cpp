#include "listing.h"

namespace z80 {

Listing::Listing(const std::string& filename) : filename_(filename) {
    // TODO: Open file
}

void Listing::writeLine(const std::string& line) {
    // TODO: Write formatted line
}

void Listing::close() {
    // TODO: Close file
}

} // namespace z80

#include "../src/assembler/z80_instructions.h"
#include <iostream>

int main() {
    z80::Z80Instructions instructions;
    
    std::cout << "Total instruction variants: " << instructions.getInstructionCount() << std::endl;
    
    return 0;
}

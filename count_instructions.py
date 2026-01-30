#!/usr/bin/env python3
"""Count total number of Z80 instructions in the instruction table."""

import sys
sys.path.insert(0, '/home/olliy/projects/z80_asm/build/src/assembler')

# Simple C++ instruction counter by parsing the file
def count_instructions():
    with open('/home/olliy/projects/z80_asm/src/assembler/z80_instructions.cpp', 'r') as f:
        content = f.read()
    
    # Count instructions_.push_back() calls
    # But we need to account for loops that generate multiple instructions
    
    # Parse the file and estimate based on push_back calls and loops
    lines = content.split('\n')
    
    total = 0
    in_loop = False
    loop_iterations = 0
    
    for i, line in enumerate(lines):
        # Detect loops
        if 'for (int' in line or 'for (const' in line or 'while' in line:
            # Try to determine loop count
            if 'reg = 0; reg < 8' in line or 'reg < 8' in line:
                loop_iterations = 8
                in_loop = True
            elif 'reg = 0; reg < 7' in line or 'reg < 7' in line:
                loop_iterations = 7
                in_loop = True
            elif 'dst = 0; dst < 8' in line:
                loop_iterations = 8
                in_loop = True
            elif 'bit = 0; bit < 8' in line:
                loop_iterations = 8
                in_loop = True
            elif 'cc = 0; cc < 8' in line:
                loop_iterations = 8
                in_loop = True
            elif 'cc = 0; cc < 4' in line:
                loop_iterations = 4
                in_loop = True
            elif 'i = 0; i < 2' in line or 'prefix = 0; prefix < 2' in line:
                # IX/IY loop
                if 'prefix' in line:
                    loop_iterations = 2
                    in_loop = True
        
        # Count push_back in loops
        if 'instructions_.push_back' in line:
            if in_loop and loop_iterations > 0:
                # Check if we're in nested loop
                # Look back to find nested loops
                nested_count = 1
                for j in range(max(0, i-50), i):
                    if 'for (' in lines[j] and '{' in lines[j]:
                        if 'src = 0; src < 8' in lines[j]:
                            nested_count *= 8
                        elif 'src = 0; src < 7' in lines[j]:
                            nested_count *= 7
                
                if nested_count > 1:
                    total += nested_count
                else:
                    total += loop_iterations
            else:
                total += 1
        
        # Exit loop
        if in_loop and line.strip().startswith('}'):
            in_loop = False
            loop_iterations = 0
    
    return total

if __name__ == '__main__':
    count = count_instructions()
    print(f"Estimated instruction count: {count}")
    print("\nNote: This is a rough estimate. Some instructions may be counted multiple times")
    print("or missed due to complex loop structures.")

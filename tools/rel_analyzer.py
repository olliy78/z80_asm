#!/usr/bin/env python3
"""
REL File Analyzer - Decode Microsoft Relocatable Object Module Format
Analyzes .REL files created by M80 assembler

Usage: python3 rel_analyzer.py <file.rel>
"""

import sys
import os

class BitStreamReader:
    """Reads bits from MSB-first packed bytes"""
    def __init__(self, data):
        self.data = data
        self.byte_pos = 0
        self.bit_pos = 7  # Start at MSB
        
    def read_bit(self):
        """Read one bit (MSB-first order)"""
        if self.byte_pos >= len(self.data):
            return None
        
        bit = (self.data[self.byte_pos] >> self.bit_pos) & 1
        self.bit_pos -= 1
        
        if self.bit_pos < 0:
            self.byte_pos += 1
            self.bit_pos = 7
            
        return bit
    
    def read_bits(self, count):
        """Read multiple bits, returns integer"""
        value = 0
        for i in range(count):
            bit = self.read_bit()
            if bit is None:
                return None
            value = (value << 1) | bit
        return value
    
    def read_byte(self):
        """Read 8 bits as byte"""
        return self.read_bits(8)
    
    def read_word(self):
        """Read 16 bits as word (little-endian in bitstream)"""
        low = self.read_byte()
        high = self.read_byte()
        if low is None or high is None:
            return None
        return (high << 8) | low
    
    def get_position(self):
        """Return current position as (byte, bit)"""
        return (self.byte_pos, self.bit_pos)


def decode_special_item(reader, control_code):
    """Decode a special link item based on control code"""
    
    # Items with name field only
    if control_code in [0, 1, 2, 3, 4]:
        names = {
            0: "Entry Symbol",
            1: "Select Common",
            2: "Program Name",
            3: "Request Library",
            4: "Extension Link"
        }
        name_len = reader.read_bits(3)
        name = ''
        for _ in range(name_len):
            ch = reader.read_byte()
            if ch is not None:
                name += chr(ch)
        return f"{names[control_code]}: '{name}'"
    
    # Items with value + name field
    elif control_code in [5, 6, 7]:
        names = {
            5: "Define Common Size",
            6: "Chain External",
            7: "Define Entry Point"
        }
        addr_type = reader.read_bits(2)
        addr_types = {0: "Absolute", 1: "Program Rel", 2: "Data Rel", 3: "Common Rel"}
        value = reader.read_word()
        name_len = reader.read_bits(3)
        name = ''
        for _ in range(name_len):
            ch = reader.read_byte()
            if ch is not None:
                name += chr(ch)
        return f"{names[control_code]}: '{name}' = 0x{value:04X} ({addr_types.get(addr_type, '?')})"
    
    # Unused
    elif control_code == 8:
        return "Unused (8)"
    
    # Items with value field only
    elif control_code in [9, 10, 11, 12, 13, 14]:
        names = {
            9: "External Plus Offset",
            10: "Define Data Size",
            11: "Set Location Counter",
            12: "Chain Address",
            13: "Define Program Size",
            14: "End Module"
        }
        addr_type = reader.read_bits(2)
        addr_types = {0: "Absolute", 1: "Program Rel", 2: "Data Rel", 3: "Common Rel"}
        value = reader.read_word()
        if control_code == 14 and value != 0:
            return f"{names[control_code]} with start address 0x{value:04X} ({addr_types.get(addr_type, '?')})"
        return f"{names[control_code]}: 0x{value:04X} ({addr_types.get(addr_type, '?')})"
    
    # End file
    elif control_code == 15:
        return "End File"
    
    return f"Unknown control code {control_code}"


def analyze_rel_file(filename):
    """Analyze a .REL file and decode its bitstream"""
    
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"=== REL File Analysis: {filename} ===")
    print(f"File size: {len(data)} bytes")
    print()
    
    reader = BitStreamReader(data)
    item_count = 0
    max_items = 50  # Limit output for readability
    
    while item_count < max_items:
        byte_pos, bit_pos = reader.get_position()
        
        # Read control bit
        control = reader.read_bit()
        if control is None:
            break
        
        if control == 0:
            # Absolute byte
            byte_val = reader.read_byte()
            if byte_val is None:
                break
            print(f"[{byte_pos:04X}:{bit_pos}] Absolute Byte: 0x{byte_val:02X}")
            item_count += 1
            
        elif control == 1:
            # Relocatable element or special
            type_bits = reader.read_bits(2)
            if type_bits is None:
                break
            
            if type_bits == 0:
                # Special Link Item
                control_code = reader.read_bits(4)
                if control_code is None:
                    break
                result = decode_special_item(reader, control_code)
                print(f"[{byte_pos:04X}:{bit_pos}] Special Item (Code {control_code:02d}): {result}")
                item_count += 1
                
                # Stop if End File
                if control_code == 15:
                    print("\n=== End of File ===")
                    break
                # Note if End Module (continues at byte boundary)
                elif control_code == 14:
                    print("      (Next module starts at byte boundary)")
                    
            elif type_bits == 1:
                # Program Relative
                value = reader.read_word()
                if value is None:
                    break
                print(f"[{byte_pos:04X}:{bit_pos}] Program Relative: 0x{value:04X}")
                item_count += 1
                
            elif type_bits == 2:
                # Data Relative
                value = reader.read_word()
                if value is None:
                    break
                print(f"[{byte_pos:04X}:{bit_pos}] Data Relative: 0x{value:04X}")
                item_count += 1
                
            elif type_bits == 3:
                # Common Relative
                value = reader.read_word()
                if value is None:
                    break
                print(f"[{byte_pos:04X}:{bit_pos}] Common Relative: 0x{value:04X}")
                item_count += 1
    
    if item_count >= max_items:
        print(f"\n... (stopped after {max_items} items for readability)")
    
    print()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 rel_analyzer.py <file.rel>")
        sys.exit(1)
    
    filename = sys.argv[1]
    if not os.path.exists(filename):
        print(f"Error: File '{filename}' not found")
        sys.exit(1)
    
    analyze_rel_file(filename)

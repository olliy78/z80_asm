#!/usr/bin/env python3
"""
Detailed analysis of IF/ENDIF matching with context
"""

import sys
from pathlib import Path

def analyze_file_detailed(filepath):
    """Show detailed IF/ENDIF/ELSE analysis with context"""
    print(f"\nAnalyzing: {filepath}")
    print("="*80)
    
    with open(filepath, 'r', encoding='latin-1') as f:
        lines = f.readlines()
    
    stack = []
    
    for line_num, line in enumerate(lines, 1):
        code = line.split(';')[0].strip()
        if not code:
            continue
        
        tokens = code.split()
        if not tokens:
            continue
        
        # Find conditional keyword (handle "label: IF" case)
        keyword = None
        for token in tokens:
            t = token.rstrip(':').upper()
            if t in ['IF', 'IFT', 'IFF', 'IFE', 'IF1', 'IF2', 
                     'IFDEF', 'IFNDEF', 'IFB', 'IFNB', 'IFIDN', 'IFDIF',
                     'ELSE', 'ENDIF']:
                keyword = t
                break
        
        if not keyword:
            continue
        
        indent = len(line) - len(line.lstrip())
        
        if keyword in ['IF', 'IFT', 'IFF', 'IFE', 'IF1', 'IF2', 
                       'IFDEF', 'IFNDEF', 'IFB', 'IFNB', 'IFIDN', 'IFDIF']:
            stack.append({
                'keyword': keyword,
                'line': line_num,
                'text': line.rstrip(),
                'indent': indent
            })
            print(f"{line_num:5d} {'  '*len(stack)}{keyword} {code[len(keyword):].strip()}")
            
        elif keyword == 'ELSE':
            if not stack:
                print(f"{line_num:5d} {'  '*len(stack)}>>> ERROR: ELSE without IF")
                print(f"      Line: {line.rstrip()}")
            else:
                level = len(stack)
                print(f"{line_num:5d} {'  '*level}ELSE")
                
        elif keyword == 'ENDIF':
            if not stack:
                print(f"{line_num:5d} {'  '*len(stack)}>>> ERROR: ENDIF without IF")
                print(f"      Line: {line.rstrip()}")
                # Show context
                print(f"      Previous lines:")
                for i in range(max(0, line_num-6), line_num-1):
                    print(f"      {i+1:5d}: {lines[i].rstrip()}")
            else:
                if_info = stack.pop()
                level = len(stack) + 1
                print(f"{line_num:5d} {'  '*level}ENDIF (closes {if_info['keyword']} from line {if_info['line']})")
    
    if stack:
        print(f"\n>>> UNCLOSED IFs:")
        for if_info in stack:
            print(f"  Line {if_info['line']}: {if_info['keyword']}")
            print(f"    {if_info['text']}")
    
    return len(stack) == 0

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: analyze_if_context.py <file.mac>")
        sys.exit(1)
    
    filepath = sys.argv[1]
    success = analyze_file_detailed(filepath)
    
    if success:
        print("\n✓ All IFs properly closed")
    else:
        print("\n✗ Errors found")
        sys.exit(1)

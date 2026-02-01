#!/usr/bin/env python3
"""
Analyze IF/ENDIF structure in .mac files
"""

import sys
import re
from pathlib import Path
from collections import defaultdict

class ConditionalAnalyzer:
    def __init__(self):
        self.if_keywords = [
            'IF', 'IFT', 'IFF', 'IFE', 'IF1', 'IF2',
            'IFDEF', 'IFNDEF', 'IFB', 'IFNB', 'IFIDN', 'IFDIF'
        ]
        self.stack = []
        self.errors = []
        self.stats = defaultdict(int)
        
    def analyze_file(self, filepath):
        """Analyze a single file"""
        print(f"\n{'='*80}")
        print(f"Analyzing: {filepath}")
        print(f"{'='*80}")
        
        self.stack = []
        self.errors = []
        self.stats = defaultdict(int)
        
        with open(filepath, 'r', encoding='latin-1') as f:
            lines = f.readlines()
        
        for line_num, line in enumerate(lines, 1):
            self.analyze_line(line, line_num, filepath)
        
        # Check for unclosed IFs
        if self.stack:
            for if_info in self.stack:
                self.errors.append(
                    f"Line {if_info['line']}: Unclosed {if_info['keyword']}"
                )
        
        # Print results
        self.print_results()
        
    def analyze_line(self, line, line_num, filepath):
        """Analyze a single line"""
        # Remove comments (everything after ;)
        code_part = line.split(';')[0].strip()
        
        if not code_part:
            return
        
        # Split by whitespace and get tokens
        tokens = code_part.split()
        if not tokens:
            return
        
        # Check each token for conditional directives
        # (handles cases like "label: IF condition")
        for i, token in enumerate(tokens):
            token_upper = token.rstrip(':').upper()
            
            # Check if it's a conditional directive
            if token_upper in self.if_keywords and i < len(tokens) - 1:
                # Make sure there's a condition after IF
                first_token = token_upper
                break
            elif token_upper in self.if_keywords:
                first_token = token_upper
                break
            elif token_upper in ['ELSE', 'ENDIF']:
                first_token = token_upper
                break
        else:
            return  # No conditional directive found
        
        # Check if it's a conditional directive
        if first_token in self.if_keywords:
            self.stats[first_token] += 1
            self.stack.append({
                'keyword': first_token,
                'line': line_num,
                'has_else': False
            })
            
        elif first_token == 'ELSE':
            self.stats['ELSE'] += 1
            if not self.stack:
                self.errors.append(
                    f"Line {line_num}: ELSE without matching IF\n  {line.strip()}"
                )
            else:
                if self.stack[-1]['has_else']:
                    self.errors.append(
                        f"Line {line_num}: Multiple ELSE for same IF\n  {line.strip()}"
                    )
                else:
                    self.stack[-1]['has_else'] = True
                    
        elif first_token == 'ENDIF':
            self.stats['ENDIF'] += 1
            if not self.stack:
                self.errors.append(
                    f"Line {line_num}: ENDIF without matching IF\n  {line.strip()}"
                )
            else:
                if_info = self.stack.pop()
                # Successfully matched
                
    def print_results(self):
        """Print analysis results"""
        print(f"\n--- Statistics ---")
        total_ifs = sum(self.stats.get(kw, 0) for kw in self.if_keywords)
        total_endifs = self.stats.get('ENDIF', 0)
        total_elses = self.stats.get('ELSE', 0)
        
        print(f"Total IFs:     {total_ifs}")
        for kw in sorted(self.if_keywords):
            if self.stats.get(kw, 0) > 0:
                print(f"  {kw:10s}: {self.stats[kw]}")
        print(f"Total ELSEs:   {total_elses}")
        print(f"Total ENDIFs:  {total_endifs}")
        print(f"Balance:       {total_ifs - total_endifs} (should be 0)")
        
        if self.errors:
            print(f"\n--- Errors ({len(self.errors)}) ---")
            for error in self.errors[:20]:  # Show first 20 errors
                print(error)
            if len(self.errors) > 20:
                print(f"... and {len(self.errors) - 20} more errors")
        else:
            print(f"\n✓ No errors found - all IFs matched with ENDIFs")


def analyze_with_includes(filepath, analyzer, visited=None):
    """Analyze file including all INCLUDE directives"""
    if visited is None:
        visited = set()
    
    filepath = Path(filepath).resolve()
    
    if filepath in visited:
        return
    
    visited.add(filepath)
    
    print(f"\n{'='*80}")
    print(f"Analyzing (with includes): {filepath}")
    print(f"{'='*80}")
    
    try:
        with open(filepath, 'r', encoding='latin-1') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"ERROR: Cannot read file: {e}")
        return
    
    for line_num, line in enumerate(lines, 1):
        # Check for INCLUDE directive
        code_part = line.split(';')[0].strip()
        tokens = code_part.split()
        
        if tokens and tokens[0].upper() == 'INCLUDE':
            if len(tokens) > 1:
                include_name = tokens[1]
                # Add .mac extension if not present
                if '.' not in include_name:
                    include_name = include_name.lower() + '.mac'
                
                # Try to resolve include path
                include_path = filepath.parent / include_name
                if include_path.exists():
                    print(f"  Line {line_num}: Including {include_name}")
                    analyze_with_includes(include_path, analyzer, visited)
                else:
                    print(f"  Line {line_num}: WARNING: Include file not found: {include_name}")
        
        # Analyze this line
        analyzer.analyze_line(line, line_num, filepath)
    
    # Don't print results for each file, only at the end


def main():
    if len(sys.argv) < 2:
        print("Usage: analyze_conditionals.py <file.mac> [--with-includes]")
        sys.exit(1)
    
    filepath = sys.argv[1]
    with_includes = '--with-includes' in sys.argv
    
    analyzer = ConditionalAnalyzer()
    
    if with_includes:
        # Analyze with all includes expanded
        analyze_with_includes(filepath, analyzer)
        print(f"\n{'='*80}")
        print("FINAL RESULTS (all files combined)")
        print(f"{'='*80}")
        analyzer.print_results()
    else:
        # Analyze single file
        analyzer.analyze_file(filepath)


if __name__ == '__main__':
    main()

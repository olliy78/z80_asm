#!/bin/bash
# Test cleanCpmLine logic
line="kbdsti:				;kbdst-Aufruf intern"$'\r'$'\x8a'"kbdstz:	call	kbdst1		;Taste neu gedrueckt ?"

echo "Original line length: ${#line}"
echo "Line hex:"
echo -n "$line" | od -A x -t x1z -v
echo ""
echo "Simulated cleaned (keeping everything after CR except 0x8A-0x9F):"
echo -n "$line" | sed 's/\x8a//g' | od -A x -t x1z -v

#!/bin/bash
echo "=== BIOS.MAC Analyse ==="
echo ""
echo "1. Dateigröße und Zeilen:"
wc -l doc/example/bios.mac
echo ""
echo "2. INCLUDE Statements:"
grep -i "^\s*include" doc/example/bios.mac | wc -l
grep -i "^\s*include" doc/example/bios.mac | head -10
echo ""
echo "3. Fehlende INCLUDE Files:"
for file in $(grep -i "^\s*include" doc/example/bios.mac | awk '{print $2}'); do
    if [ ! -f "doc/example/$file" ] && [ ! -f "doc/example/${file}.mac" ]; then
        echo "  FEHLT: $file"
    fi
done
echo ""
echo "4. Verwendete Direktiven:"
grep -oE "^\s*(IF|ELSE|ENDIF|MACRO|ENDM|EQU|ASET|DB|DW|DS|ORG|PUBLIC|EXTRN)" doc/example/bios.mac | sort | uniq -c | sort -rn
echo ""
echo "5. Undefined Symbols (geschätzt - Labels ohne Definition):"
# Finde alle JP/CALL Referenzen
grep -oE "(JP|CALL)\s+[a-zA-Z_][a-zA-Z0-9_]*" doc/example/bios.mac | awk '{print $2}' | sort -u > /tmp/refs.txt
# Finde alle Label-Definitionen (grob)
grep -E "^[a-zA-Z_][a-zA-Z0-9_]*:" doc/example/bios.mac | sed 's/:.*//' | sort -u > /tmp/defs.txt
echo "  Referenzen: $(wc -l < /tmp/refs.txt)"
echo "  Definitionen: $(wc -l < /tmp/defs.txt)"
echo "  Potentiell undefined (erste 10):"
comm -23 /tmp/refs.txt /tmp/defs.txt | head -10

# Z80 Assembler (M80-kompatibel)

Ein zu Microsoft M80 kompatibler Z80 Assembler in C++17.

## Überblick

Dieser Assembler erzeugt das gleiche binäre Ausgabeformat (.REL) wie M80.com und kann bestehende Z80-Projekte assemblieren. Er unterstützt den vollständigen Z80-Befehlssatz, Makros, Conditional Assembly und Multi-Module-Projekte.

## Build-Anleitung

### Voraussetzungen
- CMake 3.12+
- C++17-Compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Kompilieren

### Kompilieren

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)

# Tests ausführen
ctest --output-on-failure
```

## Verwendung

```bash
./build/bin/m80 source.mac [/L] [/S] [/X] [/Z]
```

**Optionen:**
- `/L` - Listing-Datei (.prn) erzeugen
- `/S` - Symbol-Tabelle im Listing anzeigen
- `/X` - Conditionals im Listing expandieren  
- `/Z` - Z80-Befehlssatz aktivieren (Standard)

**Ausgabe:**
- `source.rel` - Microsoft Relocatable Object Format
- `source.prn` - Listing (mit `/L`)

## Features

### Z80 Instruction Set (vollständig - 782 Varianten)
- Alle 8-bit/16-bit Load, Arithmetic, Logic Instructions
- Rotate/Shift (CB-prefix), Bit Operations (BIT/SET/RES)
- Block Instructions (LDIR, CPIR, INI, OUTI, etc.)
- Indexed Addressing (IX/IY mit Displacement)
- Indexed Bit Operations (DDCB/FDCB-prefix)
- Extended I/O (IN r,(C), OUT (C),r)
- Interrupt Instructions (IM 0/1/2, RETI, RETN)
- Special Instructions (DJNZ, LD I/R, RLD, RRD, etc.)

### Assembler-Direktiven
- **Data:** DB, DW, DS
- **Segments:** CSEG, DSEG, ASEG, ORG
- **Symbols:** EQU, DEFL, PUBLIC/ENTRY, EXTRN/EXT
- **Phase:** .PHASE/.DEPHASE (relocatable code)
- **Module:** NAME/TITLE
- **Include:** INCLUDE (auto-.mac extension, recursive)

### Makros
- **MACRO/ENDM** - User-defined macros mit Parametern
- **LOCAL** - Unique labels in macros (??0001, ??0002)
- **REPT** - Repeat blocks
- **IRP/IRPC** - Indefinite repeat
- **EXITM** - Early macro exit
- Parameter substitution mit &param
- Nesting bis 100 Levels

### Conditional Assembly
- **IF/IFT, IFE/IFF** - Expression-based
- **IF1/IF2** - Pass-dependent
- **IFDEF/IFNDEF** - Symbol existence
- **IFB/IFNB** - Blank tests
- **IFIDN/IFDIF** - String comparison
- **ELSE/ENDIF** - Flow control
- Cross-file conditionals (IF in einem File, ENDIF in included file)
- Nesting bis 255 Levels

### Expression Evaluator
M80-kompatible Operator-Präzedenz (8 Levels):
- Arithmetic: `+`, `-`, `*`, `/`, `MOD`
- Bitwise: `AND`, `OR`, `XOR`, `NOT`, `SHL`, `SHR`
- Relational: `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`
- Location Counter: `$`
- Type-Tracking: Absolute/Relocatable/External

### Zahlenformate
- Decimal: `123`, `123D`
- Hexadecimal: `0ABCDh`, `0xABCD`
- Octal: `777O`, `777Q`
- Binary: `11010011B`

### Error Reporting
- **Source Location Tracking** - Korrekte Datei:Zeile auch in nested INCLUDEs
- **Warnings vs Errors** - Assembly continues mit Warnings
- M80-kompatible Warnungen (unmatched ENDIF/ELSE)

### Ausgabeformate
- **.REL** - Microsoft Relocatable Object Format
  - Special Link Items (Module Name, Sizes, PUBLIC/EXTRN)
  - Data Items (Absolute, Relocatable mit Chain Addresses)
  - MSB-first Bitstream
- **.PRN** - Listing (M80-Format: `AAAA BBBBBBBBBB Source`)
  - Page management (50 lines/page)
  - Symbol table (alphabetically sorted)

## Bekannte Einschränkungen

- Chain Address Tracking für relocatable references noch nicht vollständig
- Einige komplexe BIOS-Dateien haben undefinierte Symbole (z.B. bios.mac: `kaltst`)
- Linker (LINKMT-kompatibel) noch nicht implementiert

## Bekannte Einschränkungen

- Chain Address Tracking für relocatable references noch nicht vollständig
- Einige komplexe BIOS-Dateien haben undefinierte Symbole (z.B. bios.mac: `kaltst`)
- Linker (LINKMT-kompatibel) noch nicht implementiert

## Projektstruktur

```
z80_asm/
├── src/assembler/          # Assembler-Komponenten
│   ├── lexer.cpp          # Token-Parsing
│   ├── parser.cpp         # 2-Pass Assembly
│   ├── z80_instructions.cpp  # Instruction Set (782 Varianten)
│   ├── expression.cpp     # Expression Evaluator
│   ├── macro_processor.cpp   # MACRO/REPT/IRP/IRPC
│   ├── conditional_processor.cpp  # IF/ENDIF
│   ├── rel_writer.cpp     # .REL Output
│   └── listing.cpp        # .PRN Output
├── tests/                 # Unit & Integration Tests
└── doc/                   # Dokumentation
    ├── ARCHITECTURE.md    # Technische Dokumentation
    ├── REL_FORMAT.md      # .REL Format-Spezifikation
    └── example/           # bios.mac Test-Files
```

## Dokumentation

- [ARCHITECTURE.md](doc/ARCHITECTURE.md) - Technische Architektur
- [REL_FORMAT.md](doc/REL_FORMAT.md) - .REL Format Details
- [M80_SYNTAX.md](doc/M80_SYNTAX.md) - M80 Syntax-Referenz

## Testing

Umfangreiche Test-Suite mit 24+ Tests:
- Unit Tests (Lexer, Parser, Expression Evaluator)
- Integration Tests (.REL-Generierung, Macro Expansion)
- Validation Tests (Byte-genaue Übereinstimmung mit M80-Output)

```bash
cd build
ctest --output-on-failure
```

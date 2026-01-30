# Z80 Assembler/Linker Project

Ein zu Microsoft M80 kompatibler Z80 Assembler und LINKMT-kompatibler Linker in C++17.

## Projektziel

Entwicklung eines Assemblers, der exakt das gleiche binäre Ausgabeformat (.REL) wie M80.com erzeugt und ein Linker, der kompatibel zu linkmt.com ist. Das Projekt soll bestehende Z80-Projekte (wie bios.mac) assemblieren können.

## Status

✅ **Phase 1: Core Assembly - ABGESCHLOSSEN** 🎉

### Abgeschlossen
- [x] Projektstruktur erstellt
- [x] CMake-Build-System konfiguriert
- [x] Lexer implementiert (vollständig)
  - Token-basiertes Parsen
  - Alle M80-Zahlenformate (D, H, 0x, O, Q, B)
  - String-Literale
  - Kommentare
- [x] Parser implementiert
  - Zwei-Pass Assembly (Symbol Table Pass + Code Generation Pass)
  - Strukturierte Operanden-Analyse
  - Pattern-Matching für Instructions
  - Segment-Management (CSEG/DSEG/ASEG)
- [x] Expression Evaluator
  - Recursive Descent Parser
  - M80-kompatible Operator-Präzedenz (8 Levels)
  - Alle Operatoren: +, -, *, /, MOD, SHL, SHR, AND, OR, XOR, NOT, Relational
  - Type-Tracking (Absolute/Relocatable/External)
  - Symbol-Lookup und Location Counter ($)
- [x] Code Generation
  - DB/DW Direktiven mit Expression-Evaluation
  - Instruction Encoding (Opcodes + Operanden-Bytes)
  - Little-Endian 16-bit Werte
- [x] Symbol Table
  - Labels, EQU-Konstanten
  - Relocatable/Absolute/External Symbole
  - Segment-Zuordnung
- [x] **BitWriter (MSB-first Bit-Packing)** ✨
  - MSB-first Bitstream-Generierung
  - Bit/Byte/Word Schreibfunktionen
  - **Validiert gegen bios.rel** (Byte-genaue Übereinstimmung!)
- [x] **.REL Writer (Complete Implementation)** ✨
  - Special Link Items (Program Name, Sizes, Entry/External Symbols)
  - Data Items (Absolute, Program-Relative, Data-Relative)
  - Location Counter Management
  - Symbol Name Encoding (6-character truncation)
  - Chain Address Support (für Relocation)
  - End Module/File Markers
- [x] **Parser-REL Integration** ✨
  - Parser::writeREL() implementiert
  - Automatische Modul-Name Ableitung
  - Segment-Größen Berechnung
  - Code/Data-Gruppierung nach Segment
  - **Vollständige .REL-Datei Generierung funktionsfähig!**
- [x] Test Suite (**24+ Tests, alle bestanden** ✓)
  - Lexer Tests (alle Token-Typen)
  - Parser Tests (Basic, Directives, Segments, Advanced Operands)
  - Expression Tests (10 Testfälle)
  - DB/DW Tests (Code-Generierung)
  - BitWriter Tests (9 Testfälle, **gegen bios.rel validiert**)
  - REL Writer Tests (6 Testfälle, **BIOSMO Header Match**)
  - **End-to-End Tests (.REL-Generierung funktional)**
- [x] **Z80 Instruction Set (769 variants - VOLLSTÄNDIG)** ✨
  - 8-bit & 16-bit Load Instructions
  - Arithmetic & Logical Instructions
  - Rotate & Shift Instructions (CB prefix)
  - Bit Manipulation (BIT/SET/RES)
  - Jump, Call, Return (conditional & unconditional)
  - Stack Operations (PUSH/POP)
  - Exchange Instructions (EX)
  - **Block Instructions** (LDIR, CPIR, INI, OUTI families)
  - **Indexed Addressing** (IX/IY with displacement)
  - **Indexed Bit Operations** (DDCB/FDCB prefix - BIT/SET/RES/Rotate on IX+d/IY+d)
  - **Extended I/O** (IN r,(C), OUT (C),r)
  - I/O Instructions (IN/OUT)
  - Special Instructions (NOP, HALT, DI, EI, etc.)
- [x] **Multi-Module Support** ✨
  - PUBLIC/ENTRY directive (export symbols)
  - EXTRN/EXT directive (import symbols)
  - NAME/TITLE directive (module naming)
  - Symbol table tracks public/external flags

### In Arbeit (Phase 2)
- [ ] REL Writer integration for PUBLIC/EXTRN symbols
- [ ] Chain Address Tracking bei Relocatable References

### Geplant
- [ ] .PRN Listing Generator
- [ ] Makro-System (Phase 3)
- [ ] bios.mac Assembly (Validation)

## Build-Anleitung

### Voraussetzungen

- CMake 3.12 oder höher
- C++17-kompatibler Compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)

# Tests ausführen
./bin/test_lexer
./bin/test_parser
./bin/test_expression
./bin/test_db_dw
./bin/test_bit_writer
./bin/test_rel_writer
./bin/test_rel_output

# Oder alle Tests auf einmal
for test in bin/test_*; do echo "=== $test ==="; $test && echo "✓"; done
```

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Verwendung

```bash
# Assembler (M80-kompatibel)
./build/bin/m80 source.mac [/L] [/X] [/Z]

# Optionen:
#   /L  - Listing-Datei (.prn) erzeugen
#   /X  - Conditionals im Listing expandieren
#   /Z  - Z80-Befehlssatz aktivieren

# Ausgabe:
#   source.rel - Microsoft Relocatable Object Format
#   source.prn - Listing (optional)
```

## Projektstruktur

```
z80_asm/
├── CMakeLists.txt          # Haupt-Build-Konfiguration
├── README.md               # Diese Datei
├── doc/                    # Dokumentation
│   ├── PROJEKT_PLANUNG.md  # Detaillierte Projektplanung
│   ├── OFFENE_FRAGEN.md    # Offene Fragen und Entscheidungen
│   ├── REL_FORMAT.md       # .REL Format-Spezifikation
│   └── example/            # Beispiel-Dateien
│       ├── bios.mac        # Test-Quellcode
│       ├── bios.prn        # Referenz-Listing
│       └── bios.rel        # Referenz-.REL
├── src/                    # Quellcode
│   ├── common/             # Gemeinsame Komponenten
│   │   ├── types.h/cpp     # Basis-Typen
│   │   └── utils.h/cpp     # Hilfsfunktionen
│   ├── assembler/          # M80-Assembler
│   │   ├── main.cpp
│   │   ├── lexer.h/cpp
│   │   ├── parser.h/cpp
│   │   ├── preprocessor.h/cpp
│   │   ├── z80_instructions.h/cpp
│   │   ├── symbol_table.h/cpp
│   │   ├── expression.h/cpp
│   │   ├── listing.h/cpp
│   │   ├── bit_writer.h/cpp    # ✨ NEU
│   │   ├── rel_writer.h/cpp    # ✨ NEU
│   │   └── errors.h/cpp
│   └── linker/             # LINKMT-Linker (später)
├── tests/                  # Tests
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_parser_advanced.cpp
│   ├── test_expression.cpp
│   ├── test_db_dw.cpp
│   ├── test_bit_writer.cpp     # ✨ NEU
│   ├── test_rel_writer.cpp     # ✨ NEU
│   └── test_rel_output.cpp     # ✨ NEU (E2E)
└── build/                  # Build-Ausgabe (nicht in Git)
```

## Entwicklungsplan

Siehe [PROJEKT_PLANUNG.md](doc/PROJEKT_PLANUNG.md) für detaillierte Informationen.

### Phasen

1. **Phase 0: Setup** ✅
2. **Phase 1: Core Assembly** ✅ **ABGESCHLOSSEN!**
3. **Phase 2: Erweiterte Features** - PUBLIC/EXTRN, vollständiger Instruction-Satz
4. **Phase 3: Makros & Conditional** - MACRO/ENDM, IF/ENDIF
5. **Phase 4: Vollständige M80-Kompatibilität** - bios.mac assemblieren
6. **Phase 5: Testing & Verifikation** - Byte-genaue Übereinstimmung
7. **Phase 6: Linker** - LINKMT-kompatibel

## Features

### M80-Syntax

- [x] **Z80-Befehlssatz (769 Varianten - VOLLSTÄNDIG)** ✨
  - Alle Standard Z80 Instructions
  - Indexed Addressing (IX/IY)
  - Indexed Bit Operations (BIT/SET/RES/Rotate auf IX+d/IY+d)
  - Block Instructions (LDIR, CPIR, etc.)
  - Extended I/O (IN/OUT mit C)
  - CB-prefixed Instructions (Rotate/Shift/Bit)
  - ED-prefixed Instructions (Block/Extended IO)
  - DDCB/FDCB-prefixed Instructions (Indexed Bit Operations)
- [x] Labels und Symbole
- [x] EQU-Konstanten
- [x] DB/DW/DS (Data Definition)
- [x] ORG (Origin)
- [x] CSEG/DSEG/ASEG (Segments)
- [x] Expressions mit M80-kompatiblen Operatoren
- [x] Location Counter ($)
- [x] **PUBLIC/EXTRN (Multi-Module Support)** ✨
- [x] **NAME/TITLE (Module Naming)** ✨
- [ ] MACRO/ENDM (Phase 3)
- [ ] IF/ELSE/ENDIF (Phase 3)

### Ausgabeformate

- ✅ **Microsoft Relocatable Object Format (.REL)** - Funktionsfähig!
- [ ] **Listing (.PRN)** - Geplant

## Testing

Das Projekt enthält umfangreiche Tests die die Funktionalität validieren:

- **Unit Tests**: Einzelne Komponenten (Lexer, Parser, Expression Evaluator)
- **Integration Tests**: DB/DW Code-Generierung, REL-Datei-Erzeugung
- **Validation Tests**: BitWriter gegen bios.rel validiert (Byte-genaue Übereinstimmung)
- **End-to-End Tests**: Komplette Assembly → .REL-Datei Pipeline

Byte-by-Byte Vergleich mit Referenz-Dateien:
- `doc/example/bios.rel` - Referenz-Output von M80

## Technische Details

### .REL Format

Das .REL-Format ist ein MSB-first Bitstream-basiertes Format:

- **Control Bit**: 1 = Special Link Item, 0 = Data Item
- **Special Link Items**: Module Name, Segment Sizes, Entry/External Symbols, etc.
- **Data Items**: Absolute, Program-Relative, Data-Relative mit Relocation-Info
- **Symbol Names**: 6 Zeichen signifikant (+ 1 Typ-Bit), rad40-ähnliche Kodierung

Detaillierte Spezifikation: [REL_FORMAT.md](doc/REL_FORMAT.md)

### Expression Evaluator

Recursive Descent Parser mit 8 Präzedenz-Levels (M80-kompatibel):

1. OR
2. XOR
3. AND
4. Relational (EQ, NE, LT, LE, GT, GE)
5. Additive (+, -)
6. Multiplicative (*, /, MOD)
7. Bitwise Shift (SHL, SHR)
8. Unary (+, -, NOT)
9. Primary (Zahlen, Symbole, $, Klammern)

## Lizenz

(TODO: Lizenz festlegen)

## Kontakt

(TODO)

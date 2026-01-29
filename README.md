# Z80 Assembler/Linker Project

Ein zu Microsoft M80 kompatibler Z80 Assembler und LINKMT-kompatibler Linker in C++17.

## Projektziel

Entwicklung eines Assemblers, der exakt das gleiche binäre Ausgabeformat (.REL) wie M80.com erzeugt und ein Linker, der kompatibel zu linkmt.com ist. Das Projekt soll bestehende Z80-Projekte (wie bios.mac) assemblieren können.

## Status

🚧 **In Entwicklung - Phase 0: Setup** 🚧

- [x] Projektstruktur erstellt
- [x] CMake-Build-System konfiguriert
- [x] Grundlegende Komponenten angelegt
- [x] Lexer implementiert (Basis)
- [ ] Parser implementieren
- [ ] Z80-Instruction-Table
- [ ] .REL Writer
- [ ] .PRN Generator
- [ ] Makro-System
- [ ] Vollständige M80-Kompatibilität

## Build-Anleitung

### Voraussetzungen

- CMake 3.12 oder höher
- C++17-kompatibler Compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make
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
│   ├── REL_FORMAT.md       # .REL Format-Spezifikation (TODO)
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
│   │   ├── rel_writer.h/cpp
│   │   └── errors.h/cpp
│   └── linker/             # LINKMT-Linker (später)
├── tests/                  # Tests
│   └── test_lexer.cpp
└── build/                  # Build-Ausgabe (nicht in Git)
```

## Entwicklungsplan

Siehe [PROJEKT_PLANUNG.md](doc/PROJEKT_PLANUNG.md) für detaillierte Informationen.

### Phasen

1. **Phase 0: Setup** ✅ (aktuell)
2. **Phase 1: Minimal-Assembler** - Einfache Programme ohne Makros
3. **Phase 2: Direktiven & Ausdrücke** - Komplexere Syntax
4. **Phase 3: Makros & Conditional** - MACRO/ENDM, IF/ENDIF
5. **Phase 4: Vollständige M80-Kompatibilität** - bios.mac assemblieren
6. **Phase 5: Testing & Verifikation** - Byte-genaue Übereinstimmung
7. **Phase 6: Linker** - LINKMT-kompatibel

## Features

### M80-Syntax (geplant)

- [x] Z80-Befehlssatz (vollständig)
- [x] Labels und Symbole
- [ ] EQU/ASET
- [ ] DB/DW/DS (Data Definition)
- [ ] ORG (Origin)
- [ ] MACRO/ENDM
- [ ] IF/ELSE/ENDIF
- [ ] PUBLIC/EXTERNAL
- [ ] ASEG/CSEG/DSEG
- [ ] Und viele mehr...

### Ausgabeformate

- **Microsoft Relocatable Object Format (.REL)** - 100% byte-kompatibel
- **Listing (.PRN)** - Formatiert wie M80

## Testing

Byte-by-Byte Vergleich mit Referenz-Dateien:
- `doc/example/bios.rel` - Referenz-Output von M80
- `doc/example/bios.prn` - Referenz-Listing

## Lizenz

(TODO: Lizenz festlegen)

## Kontakt

(TODO)

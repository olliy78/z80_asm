# Architektur-Refaktorierung

## Übersicht

Die Architektur wurde refaktorisiert um eine klare Trennung der Verantwortlichkeiten (Separation of Concerns) zu erreichen.

**Aktueller Stand (30. Januar 2026)**: Phase 2 - Multi-Module Support vollständig implementiert

## Hauptkomponenten des Systems

### 1. Instruction Set (z80_instructions.h/cpp)
**Status**: ✅ VOLLSTÄNDIG - 782 Varianten
**Verantwortung**: Kompletter Z80 Instruction Set
- Alle 8-bit/16-bit Load Instructions
- Arithmetic & Logical Operations
- Rotate & Shift (CB-prefixed)
- Bit Manipulation (BIT/SET/RES)
- **Block Instructions** (LDIR, CPIR, INI, OUTI - ED-prefixed)
- **Indexed Addressing** (IX/IY mit Displacement - DD/FD-prefixed)
- **Indexed Bit Operations** (DDCB/FDCB-prefixed)
- **Extended I/O** (IN r,(C), OUT (C),r)
- **Interrupt Instructions** (IM 0/1/2, RETI, RETN)
- **Special Instructions** (DJNZ, RLD, RRD, LD I/R)
- Jump, Call, Return (alle Bedingungen)
- Stack Operations (PUSH/POP)

**Output**: InstructionInfo Structs mit Opcodes, Cycles, Addressing Modes

### 2. Parser (parser.h/cpp)
**Status**: ✅ Phase 2 Complete
**Verantwortung**: Parsen der Quelldatei und Symbol-Management
- Liest Datei
- Tokenisiert mit Lexer
- Parst Anweisungen (2-Pass Assembly)
- Erzeugt `ParsedLine` Strukturen
- Baut Symbol-Tabelle auf
- **PUBLIC/EXTRN Directive Handling** ✨
- **PHASE/DEPHASE Support** ✨
- **NAME/TITLE Directive Support** ✨
- **Source Location Tracking** für korrekte Fehlermeldungen in INCLUDEs ✨

**Features**:
- Zwei-Pass Assembly (Symbol Table → Code Generation)
- Expression Evaluation mit M80-Operator-Präzedenz
- Segment Management (CSEG/DSEG/ASEG)
- Phase Offset für relocatable Code
- Multi-Module Symbol-Tracking
- **Source Location Tracking**: Fehler/Warnungen zeigen korrekte Datei:Zeile auch in included files
  - `expandSourceWithMacros()` nimmt `sourceLocations` als Parameter
  - `addExpandedLine()` Helper für konsistentes Tracking
  - Rekursive INCLUDEs propagieren source locations korrekt

**Output**: `vector<ParsedLine>` + `SymbolTable`

## Vorher (Monolithisch)

```
Parser
├── assemble()     - Lesen, Parsen UND Code-Generierung ❌
└── writeREL()     - Auch noch REL-Datei schreiben ❌
```

**Problem:** Parser macht zu viel (lesen, parsen, generieren, schreiben)

## Nachher (Modular)

```
┌─────────────────────────────────────────────┐
│             Assembler                       │
│  (Orchestriert den gesamten Prozess)       │
└─────────────────┬───────────────────────────┘
                  │
        ┌─────────┴────────┐
        │                  │
┌───────▼──────┐  ┌────────▼────────┐
│   Parser     │  │  OutputWriter   │
│  (Parsing)   │  │  (Schreiben)    │
└──────────────┘  └────────┬────────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
         ┌────▼───┐  ┌────▼────┐  ┌───▼────┐
         │  REL   │  │ Listing │  │  HEX   │
         │ Writer │  │ Writer  │  │ Writer │
         └────────┘  └─────────┘  └────────┘
```

## Komponenten

### 1. Parser (parser.h/cpp)
**Verantwortung:** Nur das Parsen der Quelldatei
- Liest Datei
- Tokenisiert mit Lexer
- Parst Anweisungen (2-Pass)
- Erzeugt `ParsedLine` Strukturen
- Baut Symbol-Tabelle auf

**Output:** `vector<ParsedLine>` + `SymbolTable`

### 2. Symbol Table (symbol_table.h/cpp)
**Status**: ✅ Phase 2 Complete
**Verantwortung**: Symbol-Verwaltung für Multi-Module Linking
- Speichert Labels, EQU-Konstanten
- **PUBLIC Symbol Tracking** (isPublic Flag)
- **EXTERNAL Symbol Tracking** (isExternal Flag)
- Segment-Zuordnung (CSEG/DSEG/ASEG)
- Type-Tracking (Relocatable/Absolute/External)

**New APIs**:
```cpp
std::vector<const Symbol*> getPublicSymbols() const;
std::vector<const Symbol*> getExternalSymbols() const;
const std::map<std::string, Symbol>& getAllSymbols() const;
```

### 2. Assembler (assembler.h/cpp) ✨ NEU
**Verantwortung:** Orchestrierung des Assembly-Prozesses
- Ruft Parser auf
- Erstellt `AssembledModule` Struktur
- Berechnet Segment-Größen
- Bestimmt ob Modul relocatable ist
- Leitet Module-Name aus Dateinamen ab

**Output:** `AssembledModule` (Datenstruktur im RAM)

### 3. AssembledModule (assembler.h) ✨ NEU
**Struktur:** Enthält alle assemblierten Daten
```cpp
struct AssembledModule {
    std::string moduleName;
    std::vector<ParsedLine> lines;
    SymbolTable symbolTable;
    bool isRelocatable;
    Word csegSize, dsegSize, commonSize;
};
```

### 4. OutputWriter (output_writer.h/cpp) ✨ NEU
**Verantwortung:** Verschiedene Ausgabeformate schreiben

**Interface:**
```cpp
class OutputWriter {
    virtual bool write(const AssembledModule& module, 
                      const std::string& filename) = 0;
};
```

**Implementierungen:**
- `RELOutputWriter` - Microsoft .REL Format
- `ListingOutputWriter` - .PRN Listing (TODO)
- `HEXOutputWriter` - Intel HEX (TODO)

## Verwendung

### Alte API (deprecated)
```cpp
Parser parser;
parser.assemble("test.asm");
parser.writeREL("test.rel");  // ⚠️ Deprecated
```

### Neue API ✨
```cpp
// 1. Assemblieren
Assembler assembler;
auto module = assembler.assemble("test.asm");

// 2. Verschiedene Ausgabeformate erzeugen
RELOutputWriter relWriter;
relWriter.write(*module, "test.rel");

ListingOutputWriter lstWriter;
lstWriter.write(*module, "test.lst");

HEXOutputWriter hexWriter;
hexWriter.write(*module, "test.hex");
```

## Vorteile

### 1. Single Responsibility Principle
Jede Klasse hat genau eine Aufgabe:
- Parser: Parsen
- Assembler: Koordinieren
- OutputWriter: Schreiben

### 2. Testbarkeit
Jede Komponente kann einzeln getestet werden:
```cpp
// Parser-Test (ohne Output)
Parser parser;
parser.assemble("test.asm");
assert(parser.getLines().size() == 3);

// Output-Test (ohne echtes Assemblieren)
AssembledModule mockModule;
mockModule.lines = {...};
RELOutputWriter writer;
writer.write(mockModule, "test.rel");
```

### 3. Wiederverwendbarkeit
Gleiche Daten, verschiedene Ausgaben:
```cpp
auto module = assembler.assemble("bios.asm");

relWriter.write(*module, "bios.rel");  // Für Linker
lstWriter.write(*module, "bios.lst");  // Für Entwickler
hexWriter.write(*module, "bios.hex");  // Für EPROM-Brenner
```

### 4. Erweiterbarkeit
Neue Formate ohne Parser-Änderung:
```cpp
class BinaryOutputWriter : public OutputWriter {
    bool write(const AssembledModule& module, 
               const std::string& filename) override {
        // Direktes Binärformat
    }
};
```

### 5. Klare Datenflüsse
```
Source File
    ↓
[Parser] → ParsedLines + SymbolTable
    ↓
[Assembler] → AssembledModule (im RAM)
    ↓
[OutputWriter] → .REL / .LST / .HEX Dateien
```

## Migration

Die alte `Parser::writeREL()` Methode ist als `[[deprecated]]` markiert und gibt eine Compiler-Warnung aus. Sie funktioniert noch, sollte aber nicht mehr verwendet werden.

**Migrations-Schritte:**
1. `Parser parser;` → `Assembler assembler;`
2. `parser.assemble()` → `auto module = assembler.assemble()`
3. `parser.writeREL()` → `RELOutputWriter().write(*module, ...)`

## Tests

- `test_assembler_architecture.cpp` - Test der neuen Architektur
- Alle alten Tests funktionieren weiterhin ✓

## Nächste Schritte

### ✅ Abgeschlossen
1. ✅ Architektur-Refaktorierung (Assembler/OutputWriter Pattern)
2. ✅ BitWriter mit MSB-first Bit-Packing
3. ✅ REL Writer (Special Link Items + Data Items)
4. ✅ **Vollständiger Z80 Instruction Set (782 Varianten)**
5. ✅ **PUBLIC/EXTRN Multi-Module Support**
6. ✅ **PHASE/DEPHASE Relocatable Code Support**
7. ✅ **Interrupt Instructions (IM, RETI, RETN)**
8. ✅ **NAME/TITLE Directive Support**
9. ✅ **Source Location Tracking** für korrekte Fehler/Warnungen in INCLUDEs
10. ✅ **INCLUDE-Verbesserungen**:
    - Auto-.mac extension (M80-kompatibel)
    - Recursive includes mit korrektem source tracking
    - Cross-file conditional blocks (IF in einem File, ENDIF in included file)
11. ✅ **Label: IF Syntax** - Conditionals nach Labels werden erkannt
12. ✅ **ENDIF/ELSE ohne IF** - Warnungen statt Errors (M80-kompatibel)

### 🔨 In Arbeit (Phase 2 Fortsetzung)
1. [ ] REL Writer: PUBLIC/EXTRN Symbol Emission
   - getPublicSymbols() in Entry Point Items schreiben
   - getExternalSymbols() in External Symbol Items schreiben
2. [ ] Chain Address Tracking für EXTRN References
3. [ ] End-to-End Test: Multi-Module Assembly + Linking

### 📋 Geplant (Phase 3)
1. [ ] `ListingOutputWriter` implementieren (.PRN Format)
2. [ ] `HEXOutputWriter` implementieren (Intel HEX)
3. [ ] **MACRO/ENDM System**
   - Makro-Definition Storage
   - Parameter-Substitution
   - LOCAL Labels
   - REPT/IRP/IRPC
4. [ ] **Conditional Assembly (IF/ENDIF)**
5. [ ] Vollständige bios.mac Assembly

### 🎯 Langfristig (Phase 4-6)
1. [ ] LINKMT-kompatibler Linker
2. [ ] Byte-genaue M80-Kompatibilität
3. [ ] Symbol Cross-Reference (.CRF)
4. [ ] `Parser::writeREL()` komplett entfernen (Breaking Change)

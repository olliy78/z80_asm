# Assembler-Architektur

## Überblick

Der Assembler folgt einer modularen Architektur mit klarer Trennung der Verantwortlichkeiten (Separation of Concerns).

## Architektur-Diagramm

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

## Hauptkomponenten

### 1. Instruction Set (z80_instructions.h/cpp)
**Verantwortung**: Kompletter Z80 Instruction Set (782 Varianten)
- Alle 8-bit/16-bit Load, Arithmetic, Logic Instructions
- Rotate & Shift (CB-prefixed), Bit Operations (BIT/SET/RES)
- Block Instructions (LDIR, CPIR, INI, OUTI - ED-prefixed)
- Indexed Addressing (IX/IY mit Displacement - DD/FD-prefixed)
- Indexed Bit Operations (DDCB/FDCB-prefixed)
- Extended I/O, Interrupt Instructions, Special Instructions

**Output**: InstructionInfo Structs mit Opcodes, Cycles, Addressing Modes

### 2. Lexer (lexer.h/cpp)
**Verantwortung**: Token-basiertes Parsing
- Alle M80-Zahlenformate (D, H, 0x, O, Q, B)
- String-Literale, Identifiers (inkl. @, ?, $)
- Operatoren, Kommentare

**Output**: Token-Stream

### 3. Parser (parser.h/cpp)
**Verantwortung**: Zwei-Pass Assembly und Symbol-Management
- **Pass 1**: Symbol Table aufbauen, Adressen berechnen
- **Pass 2**: Code generieren, Expressions evaluieren
- Macro-Expansion (MACRO/ENDM, REPT, IRP, IRPC)
- Conditional Assembly (IF/ENDIF mit Nesting)
- INCLUDE-Handling (recursive, auto-.mac extension)
- **Source Location Tracking** für korrekte Fehler/Warnungen

**Datenstrukturen**:
```cpp
struct ParsedLine {
    std::string label;           // Optional: Label
    std::string mnemonic;        // Instruction oder Directive
    std::vector<std::string> operands;  // Operanden
    Address address;             // Adresse im Speicher
    SegmentType segment;         // CSEG/DSEG/ASEG
    std::vector<Byte> machineCode;  // Generierter Code
};

struct SourceLocation {
    std::string filename;        // Ursprungsdatei
    int lineNumber;              // Zeilennummer
};
```

**Wichtige Funktionen**:
- `expandSourceWithMacros()` - Expandiert Macros und INCLUDEs, tracked source locations
- `addExpandedLine()` - Helper für konsistentes Source Location Tracking
- `pass1()` / `pass2()` - Zwei-Pass Assembly

**Features**:
- Expression Evaluation mit M80-Operator-Präzedenz (8 Levels)
- Segment Management (CSEG/DSEG/ASEG)
- Phase Offset für relocatable Code (.PHASE/.DEPHASE)
- Multi-Module Symbol-Tracking (PUBLIC/EXTRN)
- **Source Location Tracking**: Fehler zeigen korrekte Datei:Zeile auch in nested INCLUDEs

**Output**: `vector<ParsedLine>` + `SymbolTable`

### 4. Symbol Table (symbol_table.h/cpp)
**Verantwortung**: Symbol-Verwaltung für Multi-Module Linking
- Speichert Labels, EQU-Konstanten
- PUBLIC/EXTRN Symbol Tracking (isPublic, isExternal Flags)
- Segment-Zuordnung (CSEG/DSEG/ASEG)
- Type-Tracking (Relocatable/Absolute/External)

**APIs**:
```cpp
void addSymbol(const std::string& name, const Symbol& sym);
bool hasSymbol(const std::string& name) const;
Symbol getSymbol(const std::string& name) const;
std::vector<const Symbol*> getPublicSymbols() const;
std::vector<const Symbol*> getExternalSymbols() const;
```

### 5. Expression Evaluator (expression.h/cpp)
**Verantwortung**: Expression Parsing und Evaluation
- Recursive Descent Parser mit 8 Präzedenz-Levels (M80-kompatibel)
- Unterstützt: +, -, *, /, MOD, SHL, SHR, AND, OR, XOR, NOT, Relational
- Type-Tracking für Relocatable/Absolute/External Expressions
- Symbol-Lookup und Location Counter ($)

### 6. Macro Processor (macro_processor.h/cpp)
**Verantwortung**: Macro-Expansion
- MACRO/ENDM mit Parameter-Substitution (&param)
- LOCAL labels (unique ??0001, ??0002 generation)
- REPT/IRP/IRPC (repeat macros)
- EXITM (early exit)
- Nesting bis 100 Levels

### 7. Conditional Processor (conditional_processor.h/cpp)
**Verantwortung**: Conditional Assembly
- IF/IFT, IFE/IFF (expression-based)
- IF1/IF2 (pass-dependent)
- IFDEF/IFNDEF (symbol existence)
- IFB/IFNB, IFIDN/IFDIF (string tests)
- ELSE/ENDIF mit Nesting bis 255 Levels
- Cross-file conditionals (IF in einem File, ENDIF in included file)

### 8. Output Writers

#### REL Writer (rel_writer.h/cpp)
**Verantwortung**: Microsoft .REL Format schreiben
- MSB-first Bitstream-basiert
- Special Link Items (Module Name, Sizes, PUBLIC/EXTRN Symbols)
- Data Items (Absolute, Program-Relative, Data-Relative)
- Chain Address Support für Relocation

#### Listing Writer (listing.h/cpp)
**Verantwortung**: .PRN Listing generieren
- M80-Format: `AAAA BBBBBBBBBB Source`
- Page management (50 lines/page, configurable)
- Symbol table output (alphabetically sorted, 2 columns)

#### Bit Writer (bit_writer.h/cpp)
**Verantwortung**: MSB-first Bit-Packing
- Bit/Byte/Word write functions
- Buffer management
- Validiert gegen M80-Output (byte-genaue Übereinstimmung)

## .REL Format Details

### MSB-First Bitstream

Das .REL-Format ist ein **MSB-first Bitstream**:
- Bits werden von links (MSB) nach rechts (LSB) geschrieben
- Erstes geschriebenes Bit landet in Bit 7 (0x80)
- Nach 8 Bits wird ein neues Byte begonnen

**Beispiel:**
```cpp
BitWriter writer;
writer.writeBit(1);  // Bit 7: 10000000 = 0x80
writer.writeBit(0);  // Bit 6: 10000000
writer.writeBit(1);  // Bit 5: 10100000 = 0xA0
// Nach 8 Bits: Byte wird geschrieben, neues Byte beginnt
```

### Format-Struktur

**Control Bit:**
- `1` = Special Link Item
- `0` = Data Item

**Special Link Items:**
- Program Name (Item 00)
- Select CSEG (Item 02)
- Program Size (Item 04)
- DSEG Size (Item 06)
- COMMON Size (Item 08)
- Chain External (Item 0C)
- Entry Point (Item 0E)
- End Module (Item 0E mit Special-Bit)
- End File (Item 10)

**Data Items:**
- Absolute Data (00)
- Program-Relative (01)
- Data-Relative (10)
- Common-Relative (11)
- Mit/ohne Chain Address für Relocation

Detaillierte Spezifikation: [REL_FORMAT.md](REL_FORMAT.md)

## Bekannte Probleme & Einschränkungen

### 1. Chain Address Tracking
**Problem:** Relocatable references (z.B. `LD HL,label` wo label in CSEG) benötigen Chain Addresses.
**Status:** Grundinfrastruktur vorhanden, aber nicht vollständig implementiert.
**Impact:** Multi-Module Linking könnte fehlschlagen bei komplexen Cross-References.

### 2. Undefinierte Symbole in bios.mac
**Problem:** `bios.mac` verwendet Symbole wie `kaltst`, `warmst`, die nirgendwo definiert sind.
**Analyse:** 
- Möglicherweise incomplete BIOS oder erwartet weitere Include-Dateien
- Oder es sind Placeholder für externe Module
**Workaround:** Assembly schlägt fehl, aber Source Location Tracking zeigt korrekte Fehlerposition.

### 3. Unmatched ENDIF/ELSE
**Problem:** Einige Files haben ENDIF ohne passendes IF (z.B. `bioscrt.mac:620`)
**Lösung:** Werden jetzt als **Warnings** behandelt (M80-kompatibel), Assembly continues.
**Grund:** Cross-file conditionals oder Dead Code in den Original-Files.

### 4. Label: IF Syntax
**Problem:** M80 erlaubt `label: IF condition` (conditional nach label).
**Lösung:** Parser checkt jetzt nach Label-Parsing auf Conditionals.
**Status:** Implementiert und getestet.

## Testing

### Test-Suite
- **Unit Tests:** Lexer, Parser, Expression Evaluator, Symbol Table
- **Integration Tests:** Macro Expansion, Conditional Assembly, INCLUDE Processing
- **Validation Tests:** BitWriter gegen M80-Output (byte-genau)
- **End-to-End Tests:** Komplette Assembly → .REL-Generierung

**Ausführen:**
```bash
cd build
ctest --output-on-failure
```

### Test-Files
- `tests/test_*.asm` - Assembly-Test-Cases
- `doc/example/bios.mac` - Real-World BIOS (Referenz für M80-Kompatibilität)
- `doc/example/bios.rel` - M80-generierte Referenz

## Performance-Überlegungen

### Zwei-Pass Assembly
- **Pass 1:** Schnell (nur Symbol-Collection, keine Expression-Evaluation)
- **Pass 2:** Langsamer (Expression-Evaluation, Code-Generation)
- **Optimierung:** Forward References werden in Pass 1 gesammelt, in Pass 2 aufgelöst

### Memory Usage
- `ParsedLine` Structs werden im RAM gehalten (kompletter Source)
- Bei sehr großen Files könnte das problematisch werden
- **Optimierung möglich:** Streaming-Ansatz für Pass 2

### Macro Expansion
- Macros werden inline expandiert (kann zu Code-Explosion führen)
- Nesting-Limit: 100 Levels (verhindert Endlosschleifen)
- **Trade-off:** Memory vs. Geschwindigkeit

## Erweiterungsmöglichkeiten

### Geplante Features
1. **HEX Writer** - Intel HEX Format für EPROM-Brenner
2. **Binary Writer** - Direktes Binary-Format
3. **Linker** - LINKMT-kompatibler Linker für Multi-Module-Projects
4. **Symbol Cross-Reference (.CRF)** - Zeigt wo Symbole verwendet werden

### API-Erweiterungen
```cpp
// Künftig möglich:
class BinaryOutputWriter : public OutputWriter {
    bool write(const AssembledModule& module, const std::string& filename) override;
};

class DebugInfoWriter : public OutputWriter {
    // Generiert Debug-Informationen für Emulatoren
};
```

# Architektur-Refaktorierung

## Übersicht

Die Architektur wurde refaktorisiert um eine klare Trennung der Verantwortlichkeiten (Separation of Concerns) zu erreichen.

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

1. `ListingOutputWriter` implementieren (.PRN Format)
2. `HEXOutputWriter` implementieren (Intel HEX)
3. PUBLIC/EXTRN Symbol-Handling in `AssembledModule`
4. `Parser::writeREL()` komplett entfernen (Breaking Change)

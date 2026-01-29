# Z80 Assembler/Linker Projekt - Planung und Analyse

## Projektziel
Entwicklung eines Z80 Assemblers und Linkers in C++17, der zu **m80.com** (Microsoft Macro-80 Assembler) kompatibel ist.

## Analyseergbenisse der Beispieldateien

### Eingabeformat (.mac)
- **Quelltextformat**: ASCII-Textdatei mit Z80 Assembler-Code
- **Dialekt**: Microsoft M80-Syntax mit erweiterten Features:
  - Z80-Befehlssatz (`.z80` Direktive)
  - Makros und bedingte Assemblierung (`.tfcond`)
  - Modulname-Deklaration (`name` Direktive)
  - Titel für Listing (`title` Direktive)
  - EQU und ASET für Konstanten/Variablen
  - MACRO/ENDM für Makrodefinitionen
  - ENTRY Direktive für Export-Symbole
  - `.xlist/.list` für Listing-Steuerung
  - Bedingte Assemblierung (IF/ENDIF, etc.)

### Ausgabeformat (.prn)
- **Listing-Datei**: Formatierte ASCII-Ausgabe mit:
  - Header: Titel, Assembler-Version (MACRO-80 V3.50), Datum, Seitenzahl
  - Spaltenformat:
    - Adresse (4-stellig hex)
    - Maschinencode (hex bytes)
    - Zeilennummer
    - Quelltext (mit Einrückung)
  - Seitenumbrüche mit Header-Wiederholung
  - Symbol-/Makro-Expandierung sichtbar
  - EQU-Werte werden angezeigt

### Ausgabeformat (.rel)
- **Microsoft Relocatable Object Format**
- Binäres Format mit spezieller Struktur
- Enthält:
  - Relocatable Code/Data
  - Symbol-Tabelle
  - Relocation-Informationen
  - Externe Referenzen
- Wird vom Linker (L80) weiterverarbeitet

## Projektstruktur (Vorschlag)

```
z80_asm/
├── CMakeLists.txt              # Haupt-Build-Konfiguration
├── README.md                   # Projekt-Dokumentation
├── doc/                        # Dokumentation
│   ├── PROJEKT_PLANUNG.md     # Diese Datei
│   ├── REL_FORMAT.md          # Microsoft .REL Format-Spezifikation
│   ├── M80_SYNTAX.md          # M80-Syntax-Referenz
│   └── example/               # Testdateien
│       ├── bios.mac
│       ├── bios.prn
│       └── bios.rel
├── src/                        # Quellcode
│   ├── assembler/             # Assembler-Komponenten
│   │   ├── main.cpp           # Hauptprogramm (m80-kompatibel)
│   │   ├── lexer.h/cpp        # Lexikalische Analyse
│   │   ├── parser.h/cpp       # Parser (Syntax-Analyse)
│   │   ├── preprocessor.h/cpp # Makro-Expansion, Includes
│   │   ├── z80_instructions.h/cpp # Z80-Befehlssatz-Tabelle
│   │   ├── symbol_table.h/cpp # Symbol-Verwaltung
│   │   ├── expression.h/cpp   # Ausdrucks-Evaluierung
│   │   ├── listing.h/cpp      # .prn Listing-Generator
│   │   ├── rel_writer.h/cpp   # .rel Datei-Generator
│   │   └── errors.h/cpp       # Fehlerbehandlung
│   ├── linker/                # Linker-Komponenten (optional)
│   │   ├── main.cpp           # Hauptprogramm (l80-kompatibel)
│   │   ├── rel_reader.h/cpp   # .rel Datei-Parser
│   │   ├── relocator.h/cpp    # Adress-Relocation
│   │   └── hex_writer.h/cpp   # Intel HEX Ausgabe
│   └── common/                # Gemeinsame Komponenten
│       ├── types.h            # Gemeinsame Typen
│       └── utils.h/cpp        # Hilfsfunktionen
├── tests/                      # Unit- und Integrationstests
│   ├── CMakeLists.txt
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_z80_instructions.cpp
│   └── integration/
│       └── test_bios.cpp      # Test mit bios.mac
└── build/                      # Build-Ausgabe (nicht in Git)
```

## Hauptkomponenten des Assemblers

### 1. Lexer (Lexikalische Analyse)
- **Aufgabe**: Zerlegt Eingabetext in Tokens
- **Tokens**:
  - Labels (mit/ohne ':')
  - Mnemonics (LD, ADD, JP, etc.)
  - Register (A, B, C, D, E, H, L, AF, BC, DE, HL, IX, IY, SP)
  - Direktiven (.z80, EQU, DB, DW, etc.)
  - Operatoren (+, -, *, /, MOD, SHL, SHR, AND, OR, XOR, NOT)
  - Zahlen (decimal, hex, octal, binary)
  - Strings
  - Kommentare (;)
  
### 2. Parser
- **Aufgabe**: Syntaxanalyse und AST-Erstellung
- **Zwei-Pass-Prinzip**:
  - Pass 1: Symbol-Adressen sammeln
  - Pass 2: Code generieren

### 3. Preprocessor
- **Makro-Expansion**
- **Conditional Assembly** (IF/ELSE/ENDIF)
- **Include-Dateien**
- **Listing-Steuerung** (.list/.xlist)

### 4. Symbol-Tabelle
- Labels mit Adressen
- EQU-Konstanten
- ASET-Variablen
- PUBLIC/EXTERNAL Symbole

### 5. Expression-Evaluator
- Arithmetische Ausdrücke
- Logische Operatoren
- Relocation-Unterstützung
- Forward/Backward References

### 6. Code-Generator
- Z80-Opcodes generieren
- Adressierungsmodi
- Relocation-Informationen sammeln

### 7. .REL Writer
- Microsoft Relocatable Object Format
- Muss exakt dem Format entsprechen

### 8. .PRN Generator
- Formatiertes Listing
- Maschinencode + Quelltext
- Seitenumbrüche
- Symbol-Tabelle am Ende

## Kritische Fragen für die Umsetzung

### 1. Microsoft .REL Format-Spezifikation
**Frage**: Haben wir eine vollständige Dokumentation des .REL-Formats?
- **Status**: Muss recherchiert/reverse-engineered werden
- **Kritisch**: Exakte Byte-Kompatibilität erforderlich
- **Lösungsansatz**: 
  - Analyse von bios.rel (Hex-Dump)
  - Recherche nach Microsoft Relocatable Object Format Specs
  - Vergleich mit anderen .rel Dateien

### 2. M80-Syntax-Kompatibilität
**Frage**: Welche M80-Features müssen unterstützt werden?
- **Basis-Features**:
  - [x] Z80 Befehlssatz (vollständig)
  - [x] Labels und Symbole
  - [x] EQU/ASET
  - [x] DB/DW/DS (Data Definition)
  - [x] ORG (Origin)
  - [x] END (End of Assembly)
- **Erweiterte Features**:
  - [ ] MACRO/ENDM (Makro-Definition)
  - [ ] REPT/ENDM (Wiederholungen)
  - [ ] IF/ELSE/ENDIF (Bedingte Assemblierung)
  - [ ] IRP/IRPC (Iteration über Parameter)
  - [ ] LOCAL (lokale Labels in Makros)
  - [ ] INCLUDE/INCLUDE$ (Datei-Einbindung)
- **Direktiven**:
  - [ ] .Z80 (Z80-Modus)
  - [ ] .8080 (8080-Modus)
  - [ ] .LIST/.XLIST (Listing-Steuerung)
  - [ ] .TFCOND (True/False Conditional)
  - [ ] .COND/.NOCOND
  - [ ] NAME (Modulname)
  - [ ] TITLE (Titel)
  - [ ] ENTRY/PUBLIC (Export)
  - [ ] EXTERNAL/EXTRN (Import)
  - [ ] ASEG/CSEG/DSEG (Segment-Typen)

### 3. Zwei-Pass-Assemblierung
**Frage**: Wie handhaben wir Forward-References?
- **Pass 1**: 
  - Symbole sammeln
  - Adressen berechnen (Location Counter)
  - Makros definieren
- **Pass 2**:
  - Code generieren
  - Ausdrücke auflösen
  - Relocation-Infos erstellen

### 4. Ausdrucks-Evaluierung
**Frage**: Welche Operatoren und Prioritäten hat M80?
- **Arithmetik**: +, -, *, /, MOD
- **Bitweise**: AND, OR, XOR, NOT, SHL, SHR
- **Relational**: EQ, NE, LT, LE, GT, GE
- **Spezial**: $ (Location Counter), $$ (?)
- **Priorität**: Muss M80-kompatibel sein

### 5. Relocation und Linking
**Frage**: Welche Relocation-Typen werden benötigt?
- **Absolute**: Feste Adressen
- **Relocatable**: Verschiebbare Code/Data-Segmente
- **External**: Externe Referenzen
- **Common**: Common Blocks

### 6. Listing-Format
**Frage**: Wie genau muss das .prn-Format übereinstimmen?
- **Header**: "MACRO-80 V3.50" - soll unsere Version stehen?
- **Seitenlänge**: Zeilen pro Seite?
- **Spaltenbreiten**: Exakte Formatierung?
- **Symbol-Tabelle**: Am Ende des Listings?

### 7. Fehlerbehandlung
**Frage**: Wie meldet M80 Fehler?
- Fehlercode-Nummern?
- Fehlermeldungen im Listing?
- Fehlerstatistik am Ende?
- Warnungen vs. Fehler?

### 8. Kompatibilität mit Original-Tools
**Frage**: Wie testen wir Bit-genaue Kompatibilität?
- **Vergleich**: 
  - .rel Dateien byte-weise vergleichen
  - .prn Dateien vergleichen (mit Toleranz bei Datum/Version)
- **Testfälle**:
  - bios.mac als Haupttest
  - Weitere .mac Dateien sammeln
  - Edge-Cases testen

### 9. Build-System
**Frage**: CMake-Struktur?
- **Compiler**: C++17 (g++, clang++)
- **Libraries**: 
  - Standard Library (kein Boost nötig?)
  - Optionally: CLI-Parser (z.B. args, cxxopts)
- **Testing**: 
  - Google Test?
  - Catch2?
  - Eigenes Framework?

### 10. Z80-Befehlssatz
**Frage**: Vollständige Opcode-Tabelle?
- **Standard-Opcodes**: 8080-kompatibel
- **Z80-Erweiterungen**: 
  - IX/IY-Register
  - Bit-Operationen
  - Block-Operationen
  - Erweiterte Rotationen
- **Undokumentierte Opcodes**: Unterstützen?

## Entwicklungsplan (Inkrementell)

### Phase 0: Setup (JETZT)
1. [x] Anforderungen klären
2. [ ] CMake-Projekt aufsetzen
3. [ ] Grundlegende Projektstruktur erstellen
4. [ ] .REL Format dokumentieren (Internet-Recherche)
5. [ ] bios.mac Feature-Analyse

### Phase 1: Minimal-Assembler (Einfache Programme)
**Ziel**: Einfache .mac Dateien ohne Makros assemblieren
1. [ ] Lexer (Tokens, Zahlen, Labels, Kommentare)
2. [ ] Parser (Basis-Syntax)
3. [ ] Z80-Instruction-Table (vollständig)
4. [ ] Symbol-Tabelle (Labels, EQU)
5. [ ] Zwei-Pass-Assemblierung (Basis)
6. [ ] .REL Writer (Basis-Struktur)
7. [ ] .PRN Writer (Basis-Format)
8. [ ] Test mit einfachem Programm

### Phase 2: Direktiven & Ausdrücke
**Ziel**: Komplexere Ausdrücke und Direktiven
1. [ ] Expression-Evaluator (Arithmetik, Operatoren)
2. [ ] DB/DW/DS (Data Definition)
3. [ ] ORG, ASEG/CSEG/DSEG
4. [ ] TITLE, NAME, ENTRY
5. [ ] .LIST/.XLIST
6. [ ] $ (Location Counter)
7. [ ] Test mit mittlerem Programm

### Phase 3: Makros & Conditional
**Ziel**: MACRO/ENDM, IF/ENDIF
1. [ ] Preprocessor (Makro-Expansion)
2. [ ] MACRO/ENDM/LOCAL
3. [ ] REPT/ENDM
4. [ ] IF/ELSE/ENDIF
5. [ ] IRP/IRPC
6. [ ] .TFCOND
7. [ ] Test mit bios.mac (erste Versuche)

### Phase 4: Vollständige M80-Kompatibilität
**Ziel**: bios.mac vollständig assemblieren
1. [ ] Alle fehlenden Direktiven
2. [ ] PUBLIC/EXTERNAL
3. [ ] ASET (variable Symbole)
4. [ ] Segment-Management (vollständig)
5. [ ] Relocation-Informationen (vollständig)
6. [ ] Fehlerbehandlung (wie M80)
7. [ ] .PRN Format (exakt)
8. [ ] .REL Format (byte-genau)

### Phase 5: Testing & Verifikation
**Ziel**: Byte-genaue Übereinstimmung
1. [ ] bios.mac assemblieren
2. [ ] Byte-by-Byte Vergleich: bios.rel
3. [ ] Vergleich: bios.prn (mit Toleranz bei Datum)
4. [ ] Bug-Fixes
5. [ ] Test-Suite erstellen (Python)
6. [ ] Cross-Platform Tests

### Phase 6: Linker (später)
**Ziel**: linkmt.com Kompatibilität
1. [ ] .REL Reader
2. [ ] Symbol-Resolution
3. [ ] Relocation
4. [ ] .COM Ausgabe
5. [ ] Test mit bios.rel

## Projektanforderungen (geklärt)

### 1. Zielsetzung
- ✅ **100% byte-kompatibel** zu M80.com - **ABNAHMEKRITERIUM**
- ✅ Assembler zuerst, dann Linker (kompatibel zu linkmt.com, nicht l80.com)

### 2. Feature-Umfang
- ✅ Schrittweise alle Features, die für bios.mac benötigt werden
- ✅ Analyse von bios.mac zeigt benötigte Features
- ✅ Microsoft .REL Format ist dokumentiert (Internet-Recherche)

### 3. Zusatz-Features
- ✅ Erstmal keine zusätzlichen Features
- ✅ Eventuell später nach Bedarf während der Entwicklung

### 4. Testing
- ✅ Byte-by-Byte Vergleich mit example/bios.rel
- ✅ Test-Framework: evtl. eigenes Tool in Python oder C++
- ✅ Python für Scripting/Automatisierung

### 5. Listing-Format
- ✅ Exakt wie Original (.prn Format)

### 6. Plattform
- ✅ Cross-Platform: Linux, Windows, macOS
- ✅ CMake + C++17

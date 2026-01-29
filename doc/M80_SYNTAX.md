# Microsoft MACRO-80 Assembler - Syntax-Referenz

*Basierend auf der Original-Dokumentation MACRO80.txt*

## Übersicht

MACRO-80 ist ein relocatable Macro-Assembler für 8080 und Z80 Systeme mit ca. 14K Speicherbedarf und über 1000 Zeilen/Minute Assembly-Rate.

## Quelldatei-Format

### Zeilenformat

```
[label[:[:]]   [operator]   [arguments]     [;comment]
```

- **Max. 132 Zeichen** pro Zeile
- **Lower Case**: Wird in Symbolen, Opcodes, Pseudo-Ops zu Upper Case konvertiert
- **Lower Case**: Bleibt erhalten in Strings und Kommentaren
- **Line Numbers**: Müssen High-Bit gesetzt haben (EDIT-80 Format)

### Labels

- **Einfacher Colon** (`:`) - Normales Label
- **Doppelter Colon** (`::`) - PUBLIC-Label (automatisch exportiert)

Beispiel:
```asm
START:      LD A,5          ; Normales Label
EXPORT::    JP START        ; PUBLIC Label (äquivalent zu ENTRY EXPORT)
```

### Operator-Auswertung

Die Reihenfolge der Auswertung:
1. 8080/Z80 Mnemonic? → Befehl assemblieren
2. Pseudo-Op? → Direktive ausführen
3. Makro-Name? → Makro expandieren
4. Sonst: Als Ausdruck behandeln (implizites `DB`)

**WICHTIG**: Ausdrücke als Statement werden wie `DB <exp>` behandelt!

## Symbole

- **Signifikante Länge**: 6 Zeichen (Rest wird ignoriert)
- **Erlaubte Zeichen**: `A-Z`, `0-9`, `?`, `@`, `_` (Unterstrich bei Microsoft-Assemblern)
- **Erstes Zeichen**: Darf keine Ziffer sein
- **Case**: Lower → Upper Case konvertiert

### Externe Symbol-Referenz

```asm
LD HL,SYMBOL##      ; ## markiert Symbol als EXTERNAL
```

## Numerische Konstanten

### Radix (Zahlenbasis)

- **Standard**: Decimal (änderbar mit `.RADIX`)
- **Range**: Radix 2 (binär) bis 16 (hexadezimal)
- **Digits > 9**: A-F für Basen > 10

### Zahlenformat-Notationen

| Notation | Basis | Beispiel | Beschreibung |
|----------|-------|----------|--------------|
| `nnn`    | Aktuell | `255` | Aktuelle Radix |
| `nnnD`   | Decimal | `255D` | Explizit Dezimal |
| `nnnH`   | Hex | `0FFH` | Hexadezimal (muss mit Ziffer beginnen!) |
| `0xnn`   | Hex | `0xFF` | Alternative Hex-Notation |
| `nnnO`   | Octal | `377O` | Oktal |
| `nnnQ`   | Octal | `377Q` | Alternative Oktal-Notation |
| `nnnB`   | Binary | `11111111B` | Binär |

**KRITISCH**: Hex-Zahlen, die mit A-F beginnen, müssen mit `0` prefixed werden: `0FFH`, nicht `FFH`!

### Overflow

- 16-Bit unsigned Werte
- Overflow wird ignoriert (nur low-order 16 Bits verwendet)

### Character Constants

**1 Zeichen**: ASCII-Wert im Low-Byte, High-Byte = 0
```asm
DB 'A'          ; = 41H
```

**2 Zeichen**: Erstes Zeichen im High-Byte, zweites im Low-Byte
```asm
DW 'AB'         ; = 4142H (41H*256 + 42H)
```

### Strings

- **Delimiter**: Einfache (`'`) oder doppelte (`"`) Quotes
- **Escaping**: Delimiter verdoppeln für literal Quote
- **Null String**: Leerer String zwischen Delimitern erlaubt

Beispiel:
```asm
DB 'He said "Hello"'        ; Einfache Quotes außen
DB "It's working"           ; Doppelte Quotes außen
DB 'Don''t do that'         ; Quote verdoppeln
```

## Expression Evaluation

### Operator-Präzedenz (Höchste → Niedrigste)

1. `NUL` - Null-Operator
2. `LOW`, `HIGH` - Byte-Isolation
3. `*`, `/`, `MOD`, `SHR`, `SHL` - Multiplikativ, Shift
4. Unary Minus (`-`)
5. `+`, `-` - Additiv
6. `EQ`, `NE`, `LT`, `LE`, `GT`, `GE` - Relational
7. `NOT` - Logisches NOT
8. `AND` - Logisches AND
9. `OR`, `XOR` - Logisches OR/XOR

**WICHTIG**: Alle Operatoren außer `+`, `-`, `*`, `/` müssen von Operanden durch **mindestens ein Leerzeichen** getrennt sein!

```asm
        LD A,5 + 3          ; OK
        LD A,5 MOD 3        ; OK (Leerzeichen erforderlich)
        LD A,5MOD3          ; FEHLER!
```

### Byte-Isolation Operatoren

- `HIGH <exp>` - Isoliert High-Byte (Bits 8-15)
- `LOW <exp>` - Isoliert Low-Byte (Bits 0-7)

**Bei relokatiblen Werten**: Werden als absolut behandelt (relativ zu 0)

### Mode-System (Relocation)

Alle Symbole haben einen **Mode**:
- **Absolute** (ASEG)
- **Program Relative** (CSEG) - Standard
- **Data Relative** (DSEG)
- **Common** (COMMON block)

#### Additions-Regeln

| Operand 1 | Operator | Operand 2 | Resultat |
|-----------|----------|-----------|----------|
| Absolute  | `+`      | Absolute  | Absolute |
| Absolute  | `+`      | Relocatable | Relocatable (gleicher Mode) |
| Relocatable | `+`    | Absolute  | Relocatable (gleicher Mode) |
| Relocatable | `+`    | Relocatable | **FEHLER** |

#### Subtraktions-Regeln

| Operand 1 | Operator | Operand 2 | Resultat |
|-----------|----------|-----------|----------|
| Relocatable | `-`    | Absolute  | Relocatable (gleicher Mode) |
| Relocatable | `-`    | Relocatable (gleicher Mode) | Absolute |
| Relocatable | `-`    | Relocatable (anderer Mode) | **FEHLER** |
| Absolute  | `-`      | Relocatable | **FEHLER** |

**Wichtig**: Jeder Zwischenschritt muss regelkonform sein!

```asm
FOO EQU BAZ - ZAZ + QUX     ; OK wenn BAZ, ZAZ im gleichen Mode
```

### Externals in Expressions

- **EXTERNAL** Werte müssen immer **2-Byte Felder** verwenden
- **Erlaubte Operationen**:
  1. External allein
  2. External + Absolute-Konstante
  3. External - Absolute-Konstante
- **NICHT erlaubt**:
  - External + External
  - External in Multiplikation/Division/etc.

## Opcodes als Operands

**Nur erstes Byte** ist gültiger Operand:

```asm
DB NOP              ; OK - 1 Byte
DB (CPI 5)          ; FEHLER - > 1 Byte
DB LXI B,LABEL1     ; FEHLER - > 1 Byte
```

Klammern optional:
```asm
DB NOP              ; OK
DB (NOP)            ; OK (äquivalent)
```

## Pseudo-Operationen

### Segment-Definitionen

#### ASEG - Absolute Segment
```asm
ASEG                    ; Wechsel zu Absolute-Segment
        ORG 8000H       ; Optionaler ORG für absolute Adresse
```
- Location Counter auf absolutes Segment
- Standard-LC = 0 (ohne ORG)

#### CSEG - Code Segment (Program Relative)
```asm
CSEG                    ; Wechsel zu Code-Segment (Standard)
```
- **Standard-Modus** beim Assembler-Start
- Relocatable (Program Relative)
- Verwendet im LINK-80 mit `/P:<addr>` Switch

#### DSEG - Data Segment (Data Relative)
```asm
DSEG                    ; Wechsel zu Data-Segment
        ORG 100H        ; Optional: LC setzen
```
- Separate Relocation für RAM-Daten
- LINK-80: `/D:<addr>` Switch

#### COMMON - Common Block
```asm
COMMON  /BLOCKNAME/     ; Named Common Block
COMMON                  ; Blank Common (kein Name)
```
- Shared Memory zwischen Modulen
- LC startet immer bei 0 (Overlay-Semantik)
- Größter definierter Block gewinnt

### Data Definition

#### DB - Define Byte
```asm
DB      'Hello',0       ; String + Null
DB      10H, 20H, 30H   ; Bytes
DB      'A'             ; Single Character
```
- Strings (≥3 Zeichen): Müssen eigenständig sein (nicht in Expressions)
- Expressions: Müssen 1-Byte Resultat ergeben
- High-Byte muss 0 oder 255 sein (sonst A-Error)

#### DC - Define Character (String mit High-Bit)
```asm
DC      'Text'          ; Letztes Zeichen hat Bit 7 gesetzt
```
- Wie DB, aber **letztes Zeichen** mit High-Bit = 1
- Null-String → Error

#### DW - Define Word
```asm
DW      1234H           ; 16-Bit Wert
DW      LABEL           ; Address
DW      START, END      ; Multiple
```
- 2-Byte (Word) Werte
- Little-Endian (Low-Byte zuerst)

#### DS - Define Space
```asm
DS      100             ; Reserve 100 Bytes
```
- **WICHTIG**: Expression muss auf Pass 1 bekannt sein!
- Sonst: V-Error (Pass 1) oder U-Error/Phase-Error (Pass 2)

### Symbol-Definition

#### EQU - Equate
```asm
SYMBOL  EQU     100H    ; Konstante
ADDR    EQU     $       ; Location Counter
```
- Weist Symbol Wert zu
- **Einmalig**: Redefinition → M-Error
- External → Error

#### SET - Variable Symbol
```asm
COUNT   SET     0       ; Variable
COUNT   SET     COUNT+1 ; Increment (OK!)
```
- Wie EQU, aber **Redefinition erlaubt**
- Keine Error bei Redefinition

### Program Control

#### ORG - Set Origin
```asm
        ORG     8000H   ; Absolute
        ORG     $+100H  ; Relativ
```
- Setzt Location Counter
- Expression muss Pass 1 bekannt sein
- Muss Absolute oder im aktuellen Mode sein

#### END - End of Assembly
```asm
        END             ; Ohne Start-Adresse
        END     START   ; Mit Start-Adresse für LINK-80
```
- Markiert Programm-Ende
- Optionale Start-Adresse für Linker

### Module-Definitionen

#### NAME - Module Name
```asm
        NAME    BIOS    ; Modul-Name
```
- Definiert Modul-Name (erste 6 Zeichen signifikant)
- Alternativ: TITLE

#### TITLE - Title + Module Name
```asm
        TITLE   'CP/M BIOS V2.2'
```
- Titel für Listing (erste Zeile jeder Seite)
- Erste 6 Zeichen = Modul-Name (falls kein NAME)
- Nur **ein** TITLE erlaubt (sonst Q-Error)

#### SUBTTL - Subtitle
```asm
        SUBTTL  'Disk I/O Routines'
```
- Untertitel (zweite Zeile Seiten-Header)
- Max. 60 Zeichen
- Beliebig viele erlaubt

### Symbol-Export/Import

#### ENTRY/PUBLIC - Export Symbol
```asm
        ENTRY   START, INIT, EXIT
        PUBLIC  START, INIT, EXIT   ; Synonym
```
- Macht Symbole für andere Module sichtbar
- Symbole müssen in aktuellem Modul definiert sein
- U-Error wenn undefiniert
- M-Error wenn External/Common-Block

Alternativ:
```asm
START::                 ; :: = automatisch PUBLIC
```

#### EXT/EXTRN - Import Symbol
```asm
        EXT     BDOS, WBOOT
        EXTRN   BDOS, WBOOT     ; Synonym
```
- Deklariert externe Symbole (in anderem Modul definiert)
- M-Error wenn im aktuellen Modul definiert

Alternativ:
```asm
        JP      BDOS##          ; ## = automatisch EXTERNAL
```

### Library-Funktionen

#### .REQUEST - Library Search
```asm
        .REQUEST LIB1, LIB2     ; Library-Namen (ohne Extension)
```
- LINK-80 sucht automatisch nach undefinierten Symbolen
- Default-Extension und Disk-Drive verwendet

#### INCLUDE - Include File (CP/M nur)
```asm
        INCLUDE MACROS.LIB
        $INCLUDE DEFS.MAC       ; Synonym
        MACLIB  COMMON.MAC      ; Synonym
```
- Fügt externe Datei ein
- Default-Extensions wie Command-Line
- **Plus-Zeichen** (+) im Listing zwischen Code und Source
- **Keine verschachtelten INCLUDEs** (O-Error)
- V-Error wenn Datei nicht gefunden

### Listing Control

#### .LIST / .XLIST
```asm
        .LIST               ; Listing einschalten
        .XLIST              ; Listing ausschalten
```
- Steuert Source/Object-Code-Ausgabe
- Default: .LIST

#### .CREF / .XCREF
```asm
        .CREF               ; Cross-Reference einschalten
        .XCREF              ; Cross-Reference ausschalten
```
- Steuert Cross-Reference-Informationen
- Default: .CREF

#### Conditional Listing

```asm
        .SFCOND             ; Suppress False Conditionals
        .LFCOND             ; List False Conditionals
        .TFCOND             ; Toggle False Conditional Listing
```

- `.SFCOND`: Unterdrückt Listing von false-Blöcken
- `.LFCOND`: Listet false-Blöcke
- `.TFCOND`: Toggle aktuellen Zustand

#### Macro Expansion Listing

```asm
        .LALL               ; List All (vollständige Makro-Expansion)
        .SALL               ; Suppress All (keine Makro-Ausgabe)
        .XALL               ; eXpanded (nur Zeilen mit Code) - DEFAULT
```

#### PAGE - Page Break
```asm
        PAGE                ; Neue Seite
        PAGE    60          ; Neue Seite + Seitenlänge = 60 Zeilen
```
- Form-Feed im Listing
- Optionale Page-Size (10-255 Zeilen)
- Default: 50 Zeilen/Seite

### Special Features

#### .COMMENT - Comment Block
```asm
        .COMMENT *
        Dies ist ein
        mehrzeiliger Kommentar
        *
```
- Erstes Nicht-Leerzeichen nach .COMMENT = Delimiter
- Block endet bei nächstem Delimiter

#### .PRINTX - Print to Console
```asm
        .PRINTX /Assembling Module 1.../
```
- Ausgabe auf Terminal während Assembly
- Erstes Nicht-Leerzeichen = Delimiter
- Nützlich für Fortschrittsanzeige

#### .RADIX - Set Default Radix
```asm
        .RADIX  16          ; Hexadezimal als Standard
        .RADIX  10          ; Zurück zu Dezimal
```
- Ändert Standard-Zahlenbasis (2-16)
- Expression in .RADIX ist **immer dezimal**

#### .Z80 / .8080 - CPU Mode
```asm
        .Z80                ; Z80-Opcodes aktivieren
        .8080               ; 8080-Opcodes (Z80-Extensions deaktiviert)
```
- Default: Automatisch je nach CPU-Typ
- Alternativ: `/Z` oder `/I` Switch

### Phase/Dephase

#### .PHASE / .DEPHASE - Relocation vor Loading
```asm
        .PHASE  100H        ; Code wird für 100H assembliert
        ; Code hier
        .DEPHASE            ; Zurück zu aktueller Position
```
- Labels haben Werte ab Phase-Adresse
- Code wird aber an aktueller Position gespeichert
- Muss später zur Phase-Adresse kopiert werden

## Conditional Assembly

### Conditional Pseudo-Ops

| Pseudo-Op | Bedingung | Beschreibung |
|-----------|-----------|--------------|
| `IF <exp>` / `IFT <exp>` | exp ≠ 0 | True wenn Expression nicht Null |
| `IFE <exp>` / `IFF <exp>` | exp = 0 | True wenn Expression Null |
| `IF1` | Pass 1 | True in Pass 1 |
| `IF2` | Pass 2 | True in Pass 2 |
| `IFDEF <symbol>` | Definiert | True wenn Symbol definiert/external |
| `IFNDEF <symbol>` | Undefiniert | True wenn Symbol nicht definiert |
| `IFB <arg>` | Blank | True wenn Argument leer (für Makros) |
| `IFNB <arg>` | Not Blank | True wenn Argument nicht leer |
| `IFIDN <arg1>,<arg2>` | Identical | True wenn Strings identisch |
| `IFDIF <arg1>,<arg2>` | Different | True wenn Strings verschieden |

### Conditional-Struktur

```asm
        IF      DEBUG
        ; Code nur wenn DEBUG ≠ 0
        ELSE
        ; Code nur wenn DEBUG = 0
        ENDIF
```

- **Beliebige Verschachtelung** erlaubt
- **ELSE** optional, nur einmal pro IF
- **ENDIF** erforderlich (sonst "Unterminated conditional" Error)
- Arguments müssen **Pass 1 bekannt** sein (sonst V-Error)

## Makros

### MACRO - Macro Definition

```asm
NAME    MACRO   param1, param2, ...
        ; Makro-Body
        ; Kann &param1, &param2 verwenden
        ENDM
```

- Makro-Namen folgen Symbol-Regeln
- Parameter werden mit `&` referenziert
- Verschachtelung nur durch Speicher begrenzt

### REPT - Repeat Block

```asm
        REPT    10          ; 10 Wiederholungen
        DB      0
        ENDM
```

- Expression = Anzahl Wiederholungen (16-Bit unsigned)
- Expression darf keine Externals/Undefineds enthalten

### IRP - Indefinite Repeat

```asm
        IRP     reg, <A,B,C,D,E,H,L>
        LD      &reg, 0
        ENDM
```

- Wiederholt für jedes Element in Argument-Liste
- Argument-Liste in `<...>` eingeschlossen
- Elemente durch Komma getrennt

### IRPC - Indefinite Repeat Character

```asm
        IRPC    char, ABCDEFGH
        DB      '&char'
        ENDM
```

- Wiederholt für jedes Zeichen im String
- Nützlich für Zeichen-basierte Iteration

### LOCAL - Local Labels

```asm
NAME    MACRO   addr
        LOCAL   SKIP
        LD      A, (addr)
        OR      A
        JR      NZ, SKIP
        LD      A, 1
SKIP:   RET
        ENDM
```

- Lokale Labels in Makros
- Jede Expansion bekommt eindeutigen Namen
- Verhindert "Duplicate label" Errors

### EXITM - Exit Macro

```asm
NAME    MACRO   val
        IF      val EQ 0
        EXITM           ; Sofortiger Abbruch
        ENDIF
        LD      A, val
        ENDM
```

- Beendet Makro-Expansion vorzeitig
- Nützlich mit Conditionals

## Assembler Switches

Switches im Command-String (nach Slash `/`):

| Switch | Funktion |
|--------|----------|
| `/L` | Erzeugt Listing-File (.PRN) |
| `/M` | Makro-Expansion im Listing |
| `/O` | Oktale Adressen im Listing |
| `/S` | Symbol-Tabelle im Listing |
| `/C` | Cross-Reference-File (.CRF) erzeugen |
| `/N` | Keine Symbol-Tabelle |
| `/P` | Zeigt Page-Ejects (Form-Feeds) |
| `/Z` | Z80-Modus (auch mit `.Z80`) |
| `/I` | 8080-Modus (auch mit `.8080`) |
| `/X` | Unterdrückt false Conditionals (wie .SFCOND) |

Beispiel:
```
*=TEST/L/S       ; Listing mit Symbol-Tabelle
```

## Fehler-Codes

| Code | Bedeutung |
|------|-----------|
| A | Address error (Wert passt nicht ins Feld) |
| C | Conditional error (IF/ELSE/ENDIF Fehler) |
| D | Double definition (Symbol mehrfach definiert) |
| M | Multiply defined (Symbol-Konflikt) |
| O | Objectionable syntax (Syntax-Fehler) |
| P | Phase error (Pass 1 ≠ Pass 2) |
| Q | Questionable (z.B. mehrere TITLEs) |
| R | Register error (Falsches Register) |
| U | Undefined symbol |
| V | Value error (Expression in Pass 1 nicht berechenbar) |

## LINK-80 Hinweise

### Command-Line Switches

- `/P:<addr>` - Program (CSEG) Origin
- `/D:<addr>` - Data (DSEG) Origin
- `/S` - Library Search Mode
- `/E` - Map-File erzeugen

### Start-Address

```asm
        END     START       ; START = Entry Point
```

Linker verwendet diese Adresse als Programm-Start.

## Best Practices

1. **Immer CSEG verwenden** (außer für absolute Adressen)
2. **PUBLIC/EXTERNAL** klar dokumentieren
3. **Symbole**: Maximal 6 signifikante Zeichen beachten
4. **Hex-Zahlen**: Mit `0` prefix wenn sie mit A-F beginnen
5. **Operatoren**: Leerzeichen bei MOD, SHL, SHR, etc.
6. **Forward References**: In Pass 1 vermeiden wo möglich
7. **Makros**: LOCAL für alle Labels
8. **Conditionals**: Sauber einrücken für Lesbarkeit

## Kompatibilitäts-Checkliste für Implementierung

- [ ] 6-Zeichen Symbol-Signifikanz
- [ ] Lower → Upper Case Konvertierung
- [ ] Alle Zahlenformate (D, H, 0x, O, Q, B)
- [ ] Mode-System (ASEG/CSEG/DSEG/COMMON)
- [ ] Expression-Präzedenz exakt
- [ ] Operator-Spacing-Regeln
- [ ] Zwei-Pass mit Forward-References
- [ ] Makro-Verschachtelung
- [ ] Conditional-Verschachtelung (255 Levels)
- [ ] Bitstream .REL-Format
- [ ] Listing-Format exakt
- [ ] Alle Fehler-Codes
- [ ] Command-Line Switches

# Offene Fragen und Klärungsbedarf

*Stand: Januar 2026 - Nach Analyse der Original-Dokumentation*

## ✅ Geklärte Aspekte

### Format und Syntax
- ✅ **.REL-Format**: Vollständig dokumentiert (Bitstream-basiert!)
- ✅ **Bit-Packing-Reihenfolge**: MSB-FIRST (verifiziert durch bios.rel-Analyse)
- ✅ **Symbol-Länge**: 6 Zeichen signifikant (verifiziert: BIOSMOD→BIOSMO)
- ✅ **M80-Syntax**: Vollständig aus MACRO80.txt extrahiert
- ✅ **Operator-Präzedenz**: Dokumentiert und spezifiziert
- ✅ **Mode-System**: ASEG/CSEG/DSEG/COMMON Regeln klar
- ✅ **Fehler-Codes**: Alle 10 Codes dokumentiert
- ✅ **Zahlenformate**: D, H, 0x, O, Q, B alle spezifiziert
- ✅ **Symbol-Regeln**: 6 Zeichen signifikant, Case-Handling

## ❓ Kritische Implementierungs-Fragen

### 1. .REL Bitstream-Details

#### Frage 1.1: Bit-Reihenfolge beim Packing ✅ GELÖST
**Problem**: In welcher Reihenfolge werden Bits in Bytes gepackt?
```
Option A (MSB first): Bit 0 → Bit 7 → Bit 6 → ... → Bit 1  ✓ KORREKT
Option B (LSB first): Bit 0 → Bit 1 → Bit 2 → ... → Bit 7  ✗
```
**Lösung**: Durch Analyse von bios.rel verifiziert
**Analyse-Ergebnis**:
```
Bytes:     0x85        0x90        0x92
Binär:     10000101    10010000    10010010
Bitstream: 1 00 0010 110 01000010 01001001 ...
Decoded:   │ │  │    │   'B'      'I'
           │ │  │    └─ Length=6
           │ │  └─ Code=2 (Program Name)
           │ └─ Type=00 (Special)
           └─ Control=1 (Relocatable)

Module Name: "BIOSMO" (erste 6 Zeichen von "BIOSMOD")
```
**Ergebnis**: **MSB-FIRST Bit-Packing**
- Bits werden von links nach rechts gelesen
- Erstes Bit im Bitstream → MSB (Bit 7) des ersten Bytes
- Achtes Bit im Bitstream → LSB (Bit 0) des ersten Bytes
- Neuntes Bit im Bitstream → MSB (Bit 7) des zweiten Bytes

**Implementierung**:
```cpp
class BitWriter {
    uint8_t current_byte = 0;
    int bit_position = 7;  // Start at MSB
    
    void writeBit(bool bit) {
        if (bit) {
            current_byte |= (1 << bit_position);
        }
        bit_position--;
        if (bit_position < 0) {
            output.write(current_byte);
            current_byte = 0;
            bit_position = 7;
        }
    }
};
```

#### Frage 1.2: Byte-Alignment nach End Module
**Problem**: Welche Bits für Padding zur Byte-Grenze?
```
Option A: Padding mit 0-Bits
Option B: Padding mit 1-Bits
Option C: Undefiniert (beliebig)
```
**Lösung**: Multi-Modul .REL analysieren
**Priorität**: HOCH
**Test**: Zwei-Modul-Datei mit nicht-Byte-alignten Grenzen

#### Frage 1.3: Chain-Adressen Format
**Problem**: Sind Chain-Adressen absolut oder segment-relativ?
```asm
EXTRN FOO
        ORG 100H
        LD HL,(FOO)     ; Adresse 0x0100 oder 0x0000?
```
**Lösung**: EXTRN-Test-Programm analysieren
**Priorität**: HOCH
**Test**: Einfaches 2-Modul-Programm mit EXTRN

#### Frage 1.4: Common Block Overlap
**Problem**: Wie behandelt LINK-80 überlappende Common-Definitionen?
```asm
; Modul A
COMMON  /BLOCK/
DATA1:  DS  10

; Modul B  
COMMON  /BLOCK/
DATA2:  DS  20      ; Größer!
```
**Lösung**: Dokumentation sagt "größter gewinnt", aber wie bei Overlap?
**Priorität**: MITTEL (für bios.mac evtl. nicht relevant)

### 2. Lexer/Parser-Details

#### Frage 2.1: Lower-Case in Line-Numbers
**Problem**: EDIT-80 Line-Numbers mit High-Bit - wie parsen?
```
Byte: 0x81 0x82 0x83 0x0D   ; Line number mit High-Bit
```
**Lösung**: Erstmal ignorieren (optional unterstützen)
**Priorität**: NIEDRIG (bios.mac hat keine Line-Numbers)

#### Frage 2.2: Tab-Expansion
**Problem**: Wie werden Tabs interpretiert?
```
Option A: Tab = 8 Spaces
Option B: Tab = nächste 8er-Grenze
```
**Lösung**: M80-Verhalten testen oder liberal parsen
**Priorität**: NIEDRIG

#### Frage 2.3: String High-Bit Encoding
**Problem**: Werden String-Zeichen als 7-Bit oder 8-Bit gespeichert?
```asm
DB 'A'      ; = 0x41 oder 0xC1?
DC 'A'      ; Letztes Zeichen = 0xC1
```
**Lösung**: Dokumentation sagt 7-Bit (High=0), außer bei DC
**Priorität**: NIEDRIG - bereits dokumentiert

### 3. Expression Evaluator

#### Frage 3.1: Overflow-Verhalten bei Multiplikation
**Problem**: Wie behandelt M80 16-Bit Overflow?
```asm
VAL EQU 256 * 256       ; = 0x0000 (Overflow) oder Error?
```
**Lösung**: Dokumentation sagt "low-order 16 Bits", also 0x0000
**Priorität**: NIEDRIG - bereits geklärt

#### Frage 3.2: Division durch Null
**Problem**: Was passiert bei `DIV 0`?
```asm
VAL EQU 100 / 0         ; Error oder 0xFFFF?
```
**Lösung**: M80-Verhalten testen
**Priorität**: MITTEL - muss definiert sein

#### Frage 3.3: MOD mit negativen Zahlen
**Problem**: Wie funktioniert MOD bei 16-Bit unsigned Interpretation?
```asm
VAL EQU -5 MOD 3        ; = ? (da -5 als 0xFFFB interpretiert)
```
**Lösung**: M80-Verhalten testen (vermutlich unsigned MOD)
**Priorität**: NIEDRIG

### 4. Macro-System

#### Frage 4.1: LOCAL Label-Generierung
**Problem**: Wie werden lokale Label-Namen generiert?
```asm
TEST    MACRO
        LOCAL   SKIP
SKIP:   NOP
        ENDM
        
        TEST        ; SKIP → ??0001?
        TEST        ; SKIP → ??0002?
```
**Lösung**: M80-Listing mit Makros analysieren
**Priorität**: MITTEL (für Phase 3)

#### Frage 4.2: Macro-Rekursion
**Problem**: Ist rekursive Makro-Expansion erlaubt?
```asm
RECURSE MACRO   n
        IF n GT 0
        DB n
        RECURSE %(n-1)%     ; Rekursiv?
        ENDIF
        ENDM
```
**Lösung**: Dokumentation schweigt - vermutlich NEIN (Stack-Limit)
**Priorität**: NIEDRIG (vorerst verbieten)

#### Frage 4.3: Macro-Parameter mit Kommas
**Problem**: Wie werden Kommas in Makro-Parametern gequotet?
```asm
CALL_MACRO  <LD HL,100>     ; Komma in Parameter?
```
**Lösung**: Angle-Brackets `<...>` für Grouping (siehe IRP)
**Priorität**: MITTEL (für Phase 3)

### 5. Listing-Format

#### Frage 5.1: Header-Format exakt
**Problem**: Exakte Spalten und Format des Headers?
```
MACRO-80 V3.50   --DATE--        PAGE  1
Title here
Subtitle here

Addr  Code      Line   Source
```
**Lösung**: bios.prn analysieren und exakt nachbilden
**Priorität**: MITTEL (funktional nicht kritisch, aber Kompatibilität)

#### Frage 5.2: Code-Spalte bei Multi-Byte
**Problem**: Wie werden mehrzeilige Opcodes dargestellt?
```
0100  21 00 01  0010   LD HL,0100H     ; Alles eine Zeile?
oder
0100  21        0010   LD HL,0100H     ; Erste Zeile
0101  00 01                            ; Fortsetzung?
```
**Lösung**: bios.prn analysieren
**Priorität**: MITTEL

#### Frage 5.3: Symbol-Tabelle Format
**Problem**: Wie ist die Symbol-Tabelle am Ende formatiert?
```
START   0100  P     ; P = Program Relative?
DATA    0020  D     ; D = Data Relative?
```
**Lösung**: bios.prn Symbol-Tabelle analysieren
**Priorität**: MITTEL

### 6. Edge Cases

#### Frage 6.1: Leere Makros
**Problem**: Ist ein leeres Makro erlaubt?
```asm
EMPTY   MACRO
        ENDM
```
**Lösung**: Vermutlich JA (generiert keinen Code)
**Priorität**: NIEDRIG

#### Frage 6.2: Symbol = Opcode-Name
**Problem**: Kann ein Symbol wie ein Opcode heißen?
```asm
NOP     EQU 5       ; Erlaubt?
        DB NOP      ; = 0x00 oder 0x05?
```
**Lösung**: M80-Verhalten testen (vermutlich Context-abhängig)
**Priorität**: NIEDRIG - zur Sicherheit verbieten

#### Frage 6.3: Segment-Wechsel während PHASE
**Problem**: Was passiert bei CSEG während .PHASE?
```asm
        .PHASE 8000H
        ; ...
        CSEG            ; Erlaubt?
```
**Lösung**: M80-Verhalten testen oder Error generieren
**Priorität**: NIEDRIG

### 7. Linker-Fragen (Phase 6)

#### Frage 7.1: LINKMT vs L80 Unterschiede
**Problem**: Was ist der Unterschied zwischen L80 und LINKMT?
**Lösung**: Recherche oder bios.mac Kommentar analysieren
**Priorität**: NIEDRIG (erst später)

#### Frage 7.2: .COM vs .HEX Ausgabe
**Problem**: Welches Format wird für ausführbare Dateien benötigt?
**Lösung**: CP/M verwendet .COM (binär, ORG 100H)
**Priorität**: NIEDRIG (erst Phase 6)

## 🔬 Empfohlene Testprogramme

### Test 1: Minimales Programm
```asm
        ORG 0
        NOP
        END
```
**Zweck**: Bitstream-Grundstruktur verstehen

### Test 2: Relocatable Minimal
```asm
        CSEG
        NOP
        END
```
**Zweck**: Program-Relative Encoding verstehen

### Test 3: External Reference
```asm
        EXTRN BDOS
        LD C,9
        CALL BDOS##
        RET
        END
```
**Zweck**: Chain-Mechanismus verstehen

### Test 4: Multiple Segments
```asm
        CSEG
CODE:   LD HL,DATA
        
        DSEG
DATA:   DS 100
        
        END CODE
```
**Zweck**: Segment-Switching und Relocation

### Test 5: Common Block
```asm
        COMMON /SHARED/
VAR1:   DS 2
VAR2:   DS 2
        END
```
**Zweck**: Common Block Encoding

## 🎯 Nächste Schritte

### Sofort (Phase 1 Vorbereitung)
1. ✅ REL_FORMAT.md aktualisieren (DONE)
2. ✅ M80_SYNTAX.md erstellen (DONE)
3. [ ] Minimales Test-Programm (Test 1) mit M80 assemblieren
4. [ ] bios.rel Hex-Dump analysieren (erste 100 Bytes)
5. [ ] BitStream-Reader/Writer-Klasse entwerfen

### Kurzfristig (Phase 1)
1. [ ] Parser-Grundgerüst (Statement-Struktur)
2. [ ] Expression-Evaluator (ohne Relocation)
3. [ ] Basis-Direktiven (ORG, EQU, DB, DW, END)
4. [ ] Test 1 erfolgreich assemblieren

### Mittelfristig (Phase 2-3)
1. [ ] Mode-System implementieren
2. [ ] .REL BitWriter mit Tests
3. [ ] Makro-System (MACRO/ENDM, REPT)
4. [ ] Test 2-4 erfolgreich assemblieren

### Langfristig (Phase 4-5)
1. [ ] bios.mac assemblieren (alle Features)
2. [ ] Byte-by-Byte Verifikation
3. [ ] Vollständige Test-Suite

## 📋 Dokumentations-Status

| Dokument | Status | Vollständigkeit |
|----------|--------|-----------------|
| PROJEKT_PLANUNG.md | ✅ Aktualisiert | 95% |
| REL_FORMAT.md | ✅ Vollständig | 90% |
| M80_SYNTAX.md | ✅ Vollständig | 95% |
| MACRO80.txt | ✅ Original | 100% |
| rel_fileformat.md | ✅ Original | 100% |

## ❓ Fragen an Entwickler/Benutzer

1. **Bit-Packing-Reihenfolge**: Können wir ein minimales Test-Programm mit M80 assemblieren, um die Bit-Reihenfolge zu verifizieren?

2. **Prioritäten**: Sollen wir mit einem eigenen .REL-Disassembler beginnen, um das Format besser zu verstehen?

3. **Test-Strategie**: Python-Script für automatische Tests (Assemble → Compare) oder manuelle Tests?

4. **Listing-Genauigkeit**: Wie wichtig ist pixel-genaue .PRN-Kompatibilität vs. funktionale Kompatibilität?

5. **Undokumentierte Features**: Wenn wir auf undokumentiertes M80-Verhalten stoßen - nachbilden oder dokumentieren und abweichen?

6. **Error Recovery**: Wie aggressiv soll Error-Recovery sein? (M80 stoppt oft früh vs. moderne Assembler sammeln alle Fehler)

7. **Extensions**: Sind moderne Extensions gewünscht? (z.B. längere Symbole mit Option, bessere Fehler-Meldungen, etc.)

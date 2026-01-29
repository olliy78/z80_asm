# Microsoft Relocatable Object Module Format (.REL)

## Übersicht

Das Microsoft .REL-Format wird von M80, RMAC und anderen CP/M-Assemblern verwendet. Es ist ein **Bitstream-Format** (nicht byte-orientiert!), das zur Minimierung der Dateigröße auf CP/M-Systemen entwickelt wurde. Die Module werden vom Linker (LINK-80, LINKMT) zu ausführbaren Programmen verlinkt.

**KRITISCH**: Das Format ist BIT-basiert, nicht Byte-basiert! Dies ist entscheidend für 100% Byte-Kompatibilität.

## Format-Struktur (Bit-Ebene)

### Grundprinzip

Das .REL-Format ist ein **Bitstream**, der sequenziell interpretiert wird. Jedes Element beginnt mit einem oder mehreren Control-Bits, die den Typ des folgenden Elements bestimmen.

### Haupt-Bitstream-Struktur

LINK-80 interpretiert den Bitstream wie folgt:

#### 1. Erstes Bit = 0: Absolutes Byte

```
Bit-Muster: 0 BBBBBBBB
            │ └─ 8 Bits: Absolutes Byte
            └─ Control-Bit
```

- Die nächsten 8 Bits werden direkt an die aktuelle Position des Location Counters geladen
- Keine Relocation erforderlich

#### 2. Erstes Bit = 1: Relokatibles Element

```
Bit-Muster: 1 TT ...
            │ └─ 2 Bits: Typ-Code
            └─ Control-Bit
```

Die 2 Typ-Bits bestimmen:

| Typ | Binär | Beschreibung | Payload |
|-----|-------|--------------|---------|
| 0   | 00    | **Special Link Item** | 4-Bit Control-Feld + opt. Daten |
| 1   | 01    | **Program Relative (CSEG)** | 16-Bit Adresse + Program-Segment-Offset |
| 2   | 10    | **Data Relative (DSEG)** | 16-Bit Adresse + Data-Segment-Offset |
| 3   | 11    | **Common Relative** | 16-Bit Adresse + Common-Block-Offset |

## Special Link Items (Typ 100...)

Special Items steuern Linker-Operationen und haben folgende Struktur:

```
Bit-Muster: 1 00 CCCC [VV AAAAAAAAAAAAAAAA] [LLL NNNN...]
            │ │  │     │  │                  │   │
            │ │  │     │  │                  │   └─ Name (8-Bit ASCII)
            │ │  │     │  │                  └─ 3-Bit Name-Länge
            │ │  │     │  └─ 16-Bit Adresse
            │ │  │     └─ 2-Bit Adress-Typ (VV)
            │ │  └─ 4-Bit Control-Code (CCCC)
            │ └─ Special-Item-Marker (00)
            └─ Control-Bit (1)
```

### Adress-Typ-Feld (2 Bits)

| Bits | Typ | Beschreibung |
|------|-----|--------------|
| 00   | Absolute | Feste Adresse |
| 01   | Program Relative | Relativ zu CSEG-Basis |
| 10   | Data Relative | Relativ zu DSEG-Basis |
| 11   | Common Relative | Relativ zu Common-Block-Basis |

### Control-Codes (4 Bits)

#### Items mit nur Name-Feld:

| Code | Binär | Name | Beschreibung |
|------|-------|------|--------------|
| 0    | 0000  | **Entry Symbol** | Symbol ist in diesem Modul definiert (für Library-Search) |
| 1    | 0001  | **Select Common** | Wählt Common-Block für nachfolgende Common-Relative Items |
| 2    | 0010  | **Program Name** | Name des relokatiblen Moduls |
| 3    | 0011  | **Request Library** | Fordert automatisches Laden von Library an (.REQUEST) |
| 4    | 0100  | **Extension Link** | Reserviert für zukünftige Erweiterungen |

#### Items mit Value + Name-Feld:

| Code | Binär | Name | Beschreibung |
|------|-------|------|--------------|
| 5    | 0101  | **Define Common Size** | Legt Größe eines Common-Blocks fest |
| 6    | 0110  | **Chain External** | Verkettete externe Referenzen (für EXTRN) |
| 7    | 0111  | **Define Entry Point** | Definiert PUBLIC-Symbol mit Wert |

| Code | Binär | Name | Beschreibung |
|------|-------|------|--------------|
| 8    | 1000  | *(Unused)* | Nicht verwendet |

#### Items mit nur Value-Feld:

| Code | Binär | Name | Beschreibung |
|------|-------|------|--------------|
| 9    | 1001  | **External Plus Offset** | Folgende 2 Bytes + Offset nach Chain-Processing |
| 10   | 1010  | **Define Data Size** | Anzahl Bytes im Data-Segment |
| 11   | 1011  | **Set Location Counter** | Setzt Location Counter auf Wert |
| 12   | 1100  | **Chain Address** | Verkettete Adress-Referenzen (Location Counter) |
| 13   | 1101  | **Define Program Size** | Anzahl Bytes im Program-Segment |
| 14   | 1110  | **End Module** | Modul-Ende; Wert ≠ 0 = Start-Adresse |
| 15   | 1111  | **End File** | Datei-Ende (keine weiteren Felder) |

### Name-Feld-Kodierung

```
Bit-Muster: LLL CCCCCCCC CCCCCCCC ...
            │   └─ 8-Bit ASCII pro Zeichen
            └─ 3 Bits: Zeichenanzahl (0-7)
```

- Maximale Symbollänge: 7 Zeichen (3 Bits = 0-7)
- **WICHTIG**: M80 verwendet nur die ersten 6 Zeichen als signifikant
- Zeichen sind 8-Bit ASCII

## Byte-Boundary

- **End Module** (Control-Code 1110): Nächstes Modul beginnt an der nächsten **Byte-Grenze**
- **End File** (Control-Code 1111): Markiert absolutes Ende, keine weitere Verarbeitung

## Chain-Mechanismus

### Chain External (Code 0110)

Verwendet für externe Symbol-Referenzen:

1. Value-Feld enthält Adresse des ersten Vorkommens
2. An dieser Adresse steht die Adresse des nächsten Vorkommens (16-Bit)
3. Kette endet mit absolutem 0-Wert
4. Linker ersetzt alle Chain-Elemente durch die aufgelöste Symbol-Adresse

Beispiel:
```
EXTRN FOO
...
LD HL,(FOO)    ; Adresse 0x0100: enthält Zeiger auf nächstes Vorkommen
...
LD A,(FOO)     ; Adresse 0x0150: enthält 0x0000 (Ende der Kette)
```

Chain im .REL:
```
Chain External: FOO, Value=0x0100 (Program Relative)
  @ 0x0100: 0x0150 (Zeiger auf nächstes Vorkommen)
  @ 0x0150: 0x0000 (Ende)
```

### Chain Address (Code 1100)

Verwendet für Location Counter ($) Referenzen:
- Analog zu Chain External
- Wird mit aktuellem LC-Wert aufgelöst

## Segment-Typen

### ASEG (Absolute Segment)
- Feste Adressen (nicht relokatibel)
- Verwendet Bit-Muster: `0 BBBBBBBB` (absolutes Byte)

### CSEG (Code Segment / Program Relative)
- Standard-Segment für Code
- Verwendet Bit-Muster: `1 01 AAAAAAAAAAAAAAAA` (16-Bit relokatibler Wert)
- Linker addiert Program-Segment-Origin

### DSEG (Data Segment / Data Relative)
- Für RAM-Daten (getrennt von ROM-Code)
- Verwendet Bit-Muster: `1 10 AAAAAAAAAAAAAAAA`
- Linker addiert Data-Segment-Origin

### COMMON (Common Blocks)
- Shared Memory zwischen Modulen (ähnlich FORTRAN COMMON)
- Verwendet Bit-Muster: `1 11 AAAAAAAAAAAAAAAA`
- Location Counter startet immer bei 0 (Overlay-Semantik)
- Linker verwendet größte definierte Größe

## Praktische Implementierungs-Hinweise

### 1. Bit-basierte I/O erforderlich ✅ VERIFIZIERT
- **NICHT** byteweise schreiben!
- Bit-Puffer implementieren (z.B. BitWriter-Klasse)
- **Bit-Reihenfolge: MSB-FIRST** (verifiziert durch Analyse von bios.rel)
  * Bits werden von links nach rechts in Byte gepackt
  * Erstes Bit → Position 7 (MSB)
  * Zweites Bit → Position 6
  * ...
  * Achtes Bit → Position 0 (LSB)
  * Byte-Layout: `[b7 b6 b5 b4 b3 b2 b1 b0]`

### 2. Byte-Grenze bei Modul-Ende
- Nach "End Module" auf nächste Byte-Grenze auffüllen
- Padding-Bits könnten 0 oder 1 sein (muss verifiziert werden)

### 3. Zwei-Pass-Assembly erforderlich
- Pass 1: Alle Symbole und Größen sammeln
- Pass 2: .REL-Datei mit vollständigen Chain-Informationen schreiben

### 4. Symbol-Tabelle
- Verfolge alle EXTRN-Symbole mit Verwendungslisten
- Verfolge alle PUBLIC-Symbole mit Werten
- Verfolge Common-Block-Größen

### 5. Verifikation
- Byte-by-Byte-Vergleich mit M80-Output
- Disassembler für .REL-Format schreiben (Debug-Tool)

## Analyse-Beispiel: bios.rel

TODO: Hex-Dump und Bit-Stream-Dekodierung von bios.rel

```
Offset  Hex     Binary          Interpretation
------  ------  --------------  ---------------------------
0x0000  85      10000101        1 00 0010 1 = Special Item, Code 2, ...
                                (Analyse folgt)
```

## Testfälle für Implementierung

1. **Minimales Programm**: `ORG 0 / NOP / END`
2. **CSEG mit Relocation**: Labels, JP/CALL
3. **DSEG**: Daten-Definitionen
4. **EXTRN/PUBLIC**: Modul-Verlinkung
5. **COMMON**: Shared Memory
6. **Vollständig**: bios.mac → bios.rel (byte-identisch)

## Referenzen

- Microsoft MACRO-80 Assembler Manual (siehe MACRO80.txt)
- Digital Research LINK-80 Programmer's Utilities Guide
- CP/M Relocatable Object Module Format Specification
- Intel Relocatable Object Module Format (ähnlich, aber unterschiedlich)

## Verifizierte Implementierungs-Details ✅

### 1. Bit-Reihenfolge: ✅ GEKLÄRT
**Analyse von bios.rel (Bytes 0x85 0x90 0x92...):**
```
Bitstream: 1 00 0010 110 01000010 01001001 01001111 01010011 01001101 01001111
           │ │  │    │   └─────────────────────────────────────────────────┘
           │ │  │    │                 "BIOSMO" (6 chars)
           │ │  │    └─ Name Length = 6 (0b110)
           │ │  └─ Control Code = 2 (Program Name)
           │ └─ Special Link Item (00)
           └─ Relocatable (1)
```

**Ergebnis**: MSB-FIRST Bit-Packing
- Bit 0 des Bitstreams → Bit 7 des ersten Bytes (MSB)
- Bit 1 des Bitstreams → Bit 6 des ersten Bytes
- ...
- Bit 7 des Bitstreams → Bit 0 des ersten Bytes (LSB)
- Bit 8 des Bitstreams → Bit 7 des zweiten Bytes (MSB)

### 2. Symbol-Länge: ✅ GEKLÄRT
**Verifiziert**: Nur erste **6 Zeichen** sind signifikant
- `BIOSMOD` wird zu `BIOSMO` (7→6 Zeichen)
- Name-Length-Feld im Bitstream: 0b110 = 6

## Offene Fragen für Implementierung

1. ~~**Bit-Reihenfolge**~~: ✅ GEKLÄRT - MSB first
2. **Padding**: Welche Bits für Byte-Alignment nach End Module? (0 oder 1?)
3. **String-Encoding**: ASCII 7-Bit oder 8-Bit? High-Bit gesetzt? (vermutlich 8-Bit normal)
4. **Chain-Offset**: Absolut oder relativ zum Segment-Start?
5. **Common-Overlap**: Wie werden überlappende Common-Bereiche behandelt?

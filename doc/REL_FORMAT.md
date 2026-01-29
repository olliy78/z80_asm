# Microsoft Relocatable Object Module Format (.REL)

## Übersicht

Das Microsoft .REL-Format wird von M80, RMAC und anderen CP/M-Assemblern verwendet. Es ist ein binäres Format für relocatable Object-Module, die vom Linker (L80, LINKMT) zu ausführbaren Programmen verlinkt werden.

## Format-Struktur

### Allgemein

- Binärformat
- Byte-orientiert
- Record-basiert (ähnlich Intel HEX)
- Jeder Record beginnt mit einem Type-Byte

### Record-Typen

Die folgenden Record-Typen sind bekannt:

| Type | Hex  | Name                    | Beschreibung |
|------|------|-------------------------|--------------|
| 00   | 0x00 | Program Relative        | Relocatable Code/Data |
| 02   | 0x02 | Data Relative           | Data-Segment |
| 04   | 0x04 | Common Relative         | Common Block |
| 06   | 0x06 | Program Name            | Modulname |
| 08   | 0x08 | Request Library         | Library-Referenz |
| 0A   | 0x0A | Extension Link          | Externe Referenz |
| 0C   | 0x0C | External Plus Offset    | Externe Ref + Offset |
| 0E   | 0x0E | Data Area Size          | Data-Segment-Größe |
| 10   | 0x10 | Set Location Counter    | Setze LC |
| 12   | 0x12 | Chain External          | Verkettete Externe |
| 14   | 0x14 | Entry Symbol            | Public Symbol |
| 16   | 0x16 | Program Size            | Code-Segment-Größe |
| 18   | 0x18 | End of Module           | Modul-Ende |

### Detaillierte Beschreibung

#### Byte-Encoding

M80 verwendet ein spezielles Bit-Packing für effiziente Speicherung:
- Bits werden in Bytes gepackt
- Spezielle Escape-Sequenzen für Steuerinformationen

#### Address Packing

Adressen werden teilweise in komprimierter Form gespeichert.

## Analyse von bios.rel

TODO: Detaillierte Analyse der Beispieldatei

```
Offset  Hex Dump                        Beschreibung
------  ------------------------------  ---------------------------
0x0000  85 90 92 53 d4 d3 53 e0 ...    [Zu analysieren]
```

## Referenzen

- Microsoft M80 Assembler Manual
- CP/M Programmer's Guide
- Digital Research Relocatable Object Module Format
- [TODO: Internet-Recherche durchführen]

## TODO

- [ ] Vollständige Format-Spezifikation recherchieren
- [ ] bios.rel byte-weise analysieren
- [ ] Record-Struktur dokumentieren
- [ ] Relocation-Mechanismus verstehen
- [ ] Symbol-Table-Format dokumentieren
- [ ] Beispiele für jeden Record-Typ

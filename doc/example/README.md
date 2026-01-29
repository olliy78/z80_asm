# Build-Verzeichnis

Dieses Verzeichnis enthält alle Build-Artefakte und Zwischendateien des CPA Workbench Projekts.

## Finale Build-Ausgabe

- **@os.com** (15KB) - Das fertige CP/A Betriebssystem
- **@os.syp** - Symbol-Datei (Symbol-Tabelle für Debugging)

## Build-Zwischendateien

### Log-Dateien
- **bios.log** - Assembler-Log mit Build-Informationen und /p:-Wert

### Objekt-Dateien (ERL/REL)
- **cpabas.erl** - CP/A BASIC Interpreter
- **ccp.erl** - Console Command Processor
- **bdos.erl** - Basic Disk Operating System
- **bios.erl** - Basic Input/Output System (aus bios.rel gelinkt)
- **bios.rel** - Relocatable Object File des BIOS

### Source-Dateien (MAC)
47 Assembler-Source-Dateien (.mac), darunter:
- **bios.mac** - Haupt-BIOS-Datei
- **bios_org.mac** - Original BIOS
- **biosorig.mac** - Original BIOS-Variante
- **biosdsk.mac** - Disk-Treiber
- **bioskbd.mac** - Tastatur-Treiber
- **bioscrt.mac** - Bildschirm-Treiber
- **cpakey.mac** - Tasten-Definitionen
- Weitere Hardware-spezifische Module

### Listing-Dateien
- **bios.prn** (539KB) - Detailliertes Assembler-Listing mit allen Makro-Expansionen

### Build-Tools
- **m80.com** - Microsoft M80 Macro Assembler
- **linkmt.com** - Microsoft LINK/MT Linker
- **cpm.exe** - CP/M Emulator für Wine

## Build-Prozess

1. **Kopieren** - Source-Dateien (.mac) und prebuilt ERL-Dateien werden hierher kopiert
2. **Assemblieren** - `m80 =bios/L` erstellt bios.rel und bios.prn (mit Listing)
3. **Re-Assemblieren** - `m80 bios.erl=bios` erstellt finale bios.erl
4. **Linken** - `linkmt @OS=cpabas,ccp,bdos,bios/p:XXXX` erstellt @os.com
5. **Fertig** - Alle Zwischendateien bleiben erhalten für Analyse

## Nützliche Dateien für Debugging

- **bios.prn** - Vollständiges Listing zur Code-Analyse
- **bios.log** - Build-Log mit Adressen und Warnungen
- **bios.rel** - Relocatable Object für detaillierte Analyse
- **@os.syp** - Symbol-Tabelle

## Aufräumen

Um das Build-Verzeichnis zu leeren:
```bash
make clean
```

Um nur @os.com neu zu bauen:
```bash
make config os
```

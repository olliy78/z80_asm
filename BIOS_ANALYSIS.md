# BIOS.MAC Problemanalyse - Vollständige Übersicht

## Aktuelle Fehler beim Assemblieren

### 1. ELSE without IF (Line 1317, 1321)
- **Ursache**: IF/INCLUDE Nesting-Problem
- **Problem**: Nach INCLUDE-Expansion stimmen Zeilennummern nicht mehr
- **Lösung**: Komplexe Architekturänderung - Source-Context-Tracking durch INCLUDE

### 2. Undefined Symbol: kaltst (und viele andere)
- **Ursache**: Fehlende INCLUDE-Dateien
- **Betroffene Symbols**: kaltst, warmst, const, conin, conout, home, seldsk, settrk, setsec, setdma, read, write, sectran, list, listst, punch, reader
- **Lösung**: INCLUDE-Dateien müssen vorhanden sein

## Strukturelle Probleme

### 3. Alle 45 INCLUDE-Dateien fehlen!
bios.mac ist nur ein Skelett. Der eigentliche BIOS-Code ist in separaten Modulen:

**System-Core:**
- BIOSCDRV - Controller/Driver Detection
- BIOSCHK - Variant Check  
- BIOSDPBM - Disk Parameter Block Master
- BIOSDPB - Disk Parameter Block (4x included!)

**I/O-Module:**
- BIOSKBD - Keyboard
- BIOSCRT - CRT/Screen
- BIOSCSIO, BIOSCSIB, BIOSCPIO, BIOSCP54, BIOSCH54, BIOSCP56 - Serial I/O variants

**Disk-Module:**
- BIOSDSK, BIOSDSKW, BIOSDSKB, BIOSDSKT, BIOSDSKP - Floppy variants
- BIOSHD - Hard Disk
- BIOSCHD - ? (HD related)

**Memory/Timing:**
- BIOSMEM - Memory management
- BIOSTIM - Timer
- BIOSMON - Monitor

**RAM-Disk/Extensions:**
- BIOSROS, BIOSREM, BIOSRMK, BIOSRKE - ROS/REM variants
- BIOSRAF, BIOSRNA - RAF/RNA variants
- BIOSRAFI, BIOSRNAI - Interrupt variants

**Cold-Start:**
- BIOSCLD1, BIOSCLD2, BIOSCLD3 - Cold start phases
- BIOSNUC - ?
- BIOSTIMC, BIOSCHDC, BIOSDSKC, etc. - "C" variants (Controller?)

**Terminal:**
- BIOSCRTC, BIOSKBDC - CRT/KBD Controller

## Fehlende/Unimplementierte Features

### 4. IRP Directive (Repeat Macro)
```asm
IRP di,<A,B,C,D>
    ; Code hier wird für di=A, di=B, di=C, di=D wiederholt
ENDM
```
- **Komplex**: Muss Parameter-Substitution in Macro-Body machen
- **Verwendung**: 3x in bios.mac (Disk-Drive Loop)

### 5. PRINTH Directive
```asm
printh <BIOS beginnt auf .......... >,biosad
```
- **Funktion**: Print während Assembly zur Console
- **Verwendung**: 1x in bios.mac (Info-Ausgabe)

### 6. Weitere potentiell fehlende Features:
- .PHASE / .DEPHASE (haben wir schon teilweise)
- MACRO / ENDM (haben wir)
- LOCAL (lokale Labels in Macros)
- REPT (Repeat N times)
- IRPC (Repeat for each character)

## Statistik

- **Zeilen**: 1585
- **INCLUDEs**: 45 (alle fehlen!)
- **IFs**: 54 (balanced mit ENDIFs)
- **ELSEs**: 7
- **Macros**: 9 (ENDM gezählt)
- **Undefined Labels**: mindestens 13

## Prioritäten für vollständige bios.mac Unterstützung

### Kritisch (ohne geht nichts):
1. ✅ INCLUDE directive (implementiert)
2. ❌ INCLUDE-Dateien beschaffen/erstellen
3. ❌ IF/INCLUDE Context-Tracking fix

### Wichtig (für vollständige Assemblierung):
4. ❌ IRP directive
5. ❌ PRINTH directive (kann ignoriert werden)
6. ❌ Forward References (2-Pass sollte das lösen, tut es aber nicht?)

### Nice-to-have:
7. ❌ LOCAL directive
8. ❌ IRPC directive
9. ❌ Bessere Error-Messages mit korrekten Zeilennummern

## Fazit

**bios.mac alleine ist nicht assemblierbar!**

Es ist ein Haupt-Include-File, das 45 weitere Module einbindet. Ohne diese
Module fehlen:
- Alle Code-Implementierungen (Kaltstart, I/O, Disk-Routinen)
- Hunderte von Label-Definitionen
- Vermutlich weitere MACRO-Definitionen

**Für einen echten Test bräuchten wir:**
1. Alle 45 INCLUDE-Dateien
2. IRP-Directive Implementation
3. IF/INCLUDE Context-Fix
4. Möglicherweise weitere unbekannte Features

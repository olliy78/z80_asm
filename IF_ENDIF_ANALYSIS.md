# Analyse der IF/ENDIF Probleme in .mac Dateien

## Zusammenfassung

Die Analyse mit `analyze_conditionals.py` hat ergeben:

### Problemdateien

1. **bioscsio.mac** - 1 ENDIF ohne IF (Zeile 338)
2. **bioskbd.mac** - 1 ENDIF ohne IF (Zeile 737)

### Root Cause

Diese Include-Dateien sind Teil einer **Multi-File Conditional Structure**:

#### Beispiel 1: bioscsio.mac

In bios.mac:
```asm
  IF cdtdc1 or cdtdtr        ; Zeile 1452
	include BIOSCSIO         ; Zeile 1454
  ENDIF                      ; Zeile 1456 <-- schließt das IF
```

Aber bioscsio.mac enthält am Ende (Zeile 338):
```asm
 ENDIF
```

Das ENDIF in bioscsio.mac sollte eigentlich das IF aus bios.mac schließen!
Aber in bios.mac gibt es AUCH ein ENDIF nach dem INCLUDE.

**Lösung**: Das ENDIF in Zeile 1456 von bios.mac sollte NICHT da sein, ODER
das ENDIF in bioscsio.mac sollte innerhalb eines lokalen IFs sein.

#### Beispiel 2: bioskbd.mac

Die letzten Zeilen von bioskbd.mac:
```asm
 IF costu
; Nutzerstrings liegen direkt vor Interruptvektoren
 ENDIF
IF                          ; Zeile 736 - IF ohne Bedingung!
	ENDIF                     ; Zeile 737 - wird nie erreicht
```

Das `IF` ohne Bedingung wird wie `IF 0` behandelt (Code übersprungen).
Das ENDIF in Zeile 737 wird daher nie erreicht und erzeugt einen "Phantom-Fehler".

## M80 Verhalten

M80 behandelt offenbar:
1. **IF ohne Bedingung** als `IF 0` (Code wird übersprungen)
2. **Include-Dateien** können IFs öffnen, die in der includierenden Datei geschlossen werden
3. **Conditional-Stack** wird über Include-Grenzen hinweg beibehalten

## Aktuelle Parser-Implementierung

Unser Parser:
- ✅ Behandelt `IF` ohne Bedingung korrekt (wie `IF 0`)
- ✅ Behandelt `label: IF condition` korrekt (nach Fix)
- ❌ Tracked Conditional-Stack NICHT über Include-Grenzen
- ❌ Gibt Errors für unbalancierte IFs in Include-Dateien

## Empfohlene Lösung

**Option 1**: Include-Dateien teilen den Conditional-Stack
- Pro: M80-kompatibel
- Con: Komplex, Include-Dateien sind nicht standalone analysierbar

**Option 2**: Warnungen statt Errors für unbalancierte IFs
- Pro: Einfach zu implementieren  
- Con: Versteckt echte Fehler

**Option 3**: Spezielle Marker für "intentionally open" IFs
- Pro: Explizit und sicher
- Con: Nicht M80-kompatibel

## Status

- ✅ INCLUDE-Direktive funktioniert (.mac Extension wird automatisch hinzugefügt)
- ✅ Label: IF Syntax funktioniert
- ⚠️  Include-übergreifende Conditionals benötigen Design-Entscheidung

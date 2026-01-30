; Test file based on bios.mac problem areas
; Tests: mnemonic-named labels, ASET with expressions, nested IFs

	ORG 8000H

; Test 1: Labels with mnemonic names (from bios.mac line 119-122)
oem	equ	0	;OEM-Geraet
cpd	equ	1	;Hardware-Variante CP/D
cpi	equ	2	;Another mnemonic name (CPI instruction exists)
ldi	equ	3	;Another mnemonic name (LDI instruction exists)

; Test 2: ASET with expressions and redefinition
diskdi	aset	0
diskdi	aset	diskdi+1	; Should be 1
diskdi	aset	diskdi*2	; Should be 2

; Test 3: Using ASET values in code
@din	aset	0
	LD A,@din
	LD B,diskdi

; Test 4: EQU with expressions
base	equ	100H
offset	equ	base+50H
total	equ	offset*2

; Test 5: Nested conditionals (simplified from bios.mac)
diska	equ	1
diskb	equ	0
diskc	equ	1

 IF diska ne 0
	LD A,1		; Should be assembled
 ENDIF

 IF diskb ne 0
	LD A,2		; Should NOT be assembled
 ENDIF

 IF diskc ne 0
	LD A,3		; Should be assembled
 ENDIF

; Test 6: More complex conditionals
 IF diska eq 1
  IF diskc eq 1
	LD HL,1234H	; Should be assembled (both true)
  ENDIF
 ENDIF

; Test 7: Using defined constants in instructions
	LD A,oem
	LD A,cpd
	LD A,cpi
	LD A,ldi

; Test 8: Using expression results
	LD HL,base
	LD HL,offset
	LD HL,total

	RET
	END

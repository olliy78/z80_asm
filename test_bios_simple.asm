; Simple test for bios.mac issues
	ORG 1000H

; Test EQU with spaces and tabs
oem	equ	0
cpd	equ	1
k8915	equ	2

; Test nested IFs
diska	equ	1
diskb	equ	0

 IF diska ne 0
	LD A,1
 ENDIF
 IF diskb ne 0
	LD A,2
 ENDIF

	END

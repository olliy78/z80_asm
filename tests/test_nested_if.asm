; Test nested IFs with INCLUDE (simulating bios.mac structure)
; This tests the "ELSE without IF" problem

	ORG 9000H

; Setup variables like bios.mac
diska	equ	1
diskb	equ	0
diskc	equ	1
diskd	equ	0

; Pattern from bios.mac lines 1307-1325
 IF diska ne 0
diskdi	aset	diska
@din	aset	0
	LD A,@din
 ENDIF

 IF diskb ne 0
diskdi	aset	diskb
@din	aset	1
	LD A,@din
 ENDIF

 IF diskc ne 0
diskdi	aset	diskc
@din	aset	2
	LD A,@din
 ENDIF

 IF diskd ne 0
diskdi	aset	diskd
@din	aset	3
	LD A,@din
 ENDIF

; Test nested with ELSE
 IF diska eq 1
	LD A,10H
 ELSE
	LD A,20H
 ENDIF

 IF diskb eq 1
	LD A,30H
 ELSE
	LD A,40H
 ENDIF

	RET
	END

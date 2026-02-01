; Test complex IF/INCLUDE nesting
	ORG 6000H

dbufsz equ 5
diskdi equ 4001

 IF dbufsz le 7
	LD A,1
	INCLUDE test_include_nested.asm
	LD A,2
 ENDIF

	RET
	END

; Test nested IFs in false branch
	ORG 7000H

outer equ 0
inner equ 0

 IF outer eq 1
	LD A,1
	 IF inner eq 1
		LD B,2
	 ENDIF
	LD C,3
 ENDIF

	RET
	END

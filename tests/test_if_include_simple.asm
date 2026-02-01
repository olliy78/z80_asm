; Simple test for IF/INCLUDE problem
	ORG 5000H

test_var equ 1

 IF test_var eq 1
	LD A,1
	INCLUDE test_include_with_endif.asm
	LD B,2
	
	RET
	END

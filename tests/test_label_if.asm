; Test label: IF directive

start: IF 1
	ld a,5
	ELSE
	ld a,10
	ENDIF

loop: IF 0
	nop
	ENDIF

end: ret

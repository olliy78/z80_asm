; Test standalone IF

start:
	ld a,5

IF
	db "Inside IF without condition"
	ENDIF

end:
	ret

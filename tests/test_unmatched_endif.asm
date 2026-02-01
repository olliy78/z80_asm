; Test ENDIF without IF als Warnung

start:
	ld a,5

; Dieses ENDIF hat kein passendes IF
ENDIF

	ld b,10
	ret

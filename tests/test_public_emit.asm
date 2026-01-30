; Module A: Exports functions via PUBLIC
; Tests PUBLIC symbol emission in REL format

        NAME    MODULA
        
        PUBLIC  FUNC1, FUNC2, DATA1
        
        ORG     0100H

; Exported function 1
FUNC1:  LD      A, 42
        RET

; Exported function 2
FUNC2:  LD      HL, DATA1
        RET

; Exported data
DATA1:  DB      'Hello'

; Internal (not exported) function
INTERNAL:
        XOR     A
        RET

        END

; Test IRP directive
; Iterate over argument list

        NAME    IRPTEST
        ORG     0100H

; Use IRP to clear multiple registers
CLEARALL:
        IRP     reg, <A,B,C,D,E,H,L>
        XOR     &reg
        ENDM
        RET

; Use IRP to load constants
INITREGS:
        IRP     reg, <A,B,C,D>
        LD      &reg, 0
        ENDM
        RET

; Use IRP with two-register operations
SAVEREGS:
        IRP     reg, <BC,DE,HL>
        PUSH    &reg
        ENDM
        RET

RESTOREREGS:
        IRP     reg, <HL,DE,BC>
        POP     &reg
        ENDM
        RET

        END

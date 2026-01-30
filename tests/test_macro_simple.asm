; Simple MACRO test with parameters
        NAME    MACTEST
        ORG     0100H

; Macro to load register with value
LOAD    MACRO   reg
        LD      &reg, 0
        ENDM

START:
        LOAD    A               ; Should expand to: LD A, 0
        LOAD    B               ; Should expand to: LD B, 0
        RET
        END

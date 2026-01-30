; Test REPT directive
; Repeat blocks multiple times

        NAME    REPTTEST
        ORG     0100H

; Use REPT to create a table
TABLE:
        REPT    10
        DB      0
        ENDM

; REPT with multiple instructions
ZEROS:
        REPT    5
        LD      (HL), 0
        INC     HL
        ENDM

; REPT with LOCAL labels
INIT:
        LD      HL, BUFFER
        REPT    8
        LOCAL   SKIP
        LD      A, (HL)
        OR      A
        JR      Z, SKIP
        XOR     A
        LD      (HL), A
SKIP:   INC     HL
        ENDM
        RET

BUFFER: DS      8

        END

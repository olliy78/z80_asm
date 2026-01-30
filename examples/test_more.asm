;
; Test more Z80 instructions
;
        ORG     100H

; Test 8-bit loads
        LD      A,42
        LD      B,10
        LD      C,20
        LD      D,E
        LD      H,L

; Test 16-bit loads
        LD      BC,1234H
        LD      DE,5678H
        LD      HL,9ABCH
        LD      SP,0F000H

; Test arithmetic
        ADD     A,B
        ADC     A,C
        SUB     D
        SBC     A,E
        AND     H
        OR      L
        XOR     A
        CP      42

; Test INC/DEC
        INC     A
        INC     B
        DEC     C
        DEC     D
        INC     BC
        INC     DE
        DEC     HL
        DEC     SP

; Test jumps
        JP      LABEL1
        JR      LABEL2
        CALL    LABEL3
        RET

LABEL1: NOP
LABEL2: NOP
LABEL3: NOP

        END

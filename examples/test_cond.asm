;
; Test conditional jumps and calls
;
        ORG     100H

START:  XOR     A               ; A = 0, Z flag set
        JP      Z,ZERO          ; Should jump
        JP      FAIL

ZERO:   OR      A               ; Check if still zero
        JR      Z,CONT          ; Relative jump
        JP      FAIL

CONT:   LD      A,1             ; A = 1, Z flag clear
        JP      NZ,NOTZERO      ; Should jump
        JP      FAIL

NOTZERO: CP      1              ; A == 1?
        RET     Z               ; Return if equal
        
FAIL:   RET

        END     START

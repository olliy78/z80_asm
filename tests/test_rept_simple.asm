; Simple REPT test
        NAME    REPTTEST
        ORG     0100H

; Use REPT to generate NOPs
START:
        REPT    5
        NOP
        ENDM
        
        RET
        END

; Module B: Imports functions via EXTRN
; Tests EXTRN symbol emission in REL format

        NAME    MODULB
        
        EXTRN   FUNC1, FUNC2, DATA1
        
        ORG     0200H

START:  CALL    FUNC1           ; Call external function
        CALL    FUNC2           ; Call another external
        LD      HL, DATA1       ; Reference external data
        RET

        END

; Test EXITM directive
; Early exit from macro expansion

        NAME    EXITMTEST
        ORG     0100H

; Macro that uses EXITM to skip when value is 0
LOADIF  MACRO   val
        ;;; IF val EQ 0     ; Would need conditional assembly
        ;;; EXITM           ; Skip if zero
        ;;; ENDIF
        LD      A, val
        ENDM

; Simple EXITM test (unconditional for now)
TEST1   MACRO
        LD      A, 1
        EXITM               ; Exit immediately
        LD      A, 2        ; This should not be expanded
        ENDM

; Test invocations
START:
        LOADIF  5
        TEST1               ; Should only expand "LD A, 1"
        RET

        END

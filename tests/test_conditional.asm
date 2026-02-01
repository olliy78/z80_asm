; Test conditional assembly directives
; Tests IF, IFE, IF1, IF2, IFDEF, IFNDEF, ELSE, ENDIF

        ORG 1000H
        
; Test IF/ELSE/ENDIF with true condition
        IF 1
        LD A,10H        ; Should be assembled
        ELSE
        LD A,20H        ; Should be skipped
        ENDIF
        
; Test IF with false condition
        IF 0
        LD B,30H        ; Should be skipped
        ENDIF
        
; Test IFE (if equal to zero) - false
        IFE 1
        LD C,40H        ; Should be skipped
        ENDIF
        
; Test IFE - true
        IFE 0
        LD D,50H        ; Should be assembled
        ENDIF
        
; Test nested conditionals
        IF 1
        LD E,60H        ; Assembled (outer true)
        IF 0
        LD H,70H        ; Skipped (inner false)
        ELSE
        LD L,80H        ; Assembled (inner else)
        ENDIF
        LD A,90H        ; Assembled (outer true)
        ENDIF
        
; Test IFDEF
TEST_SYM    EQU 1234H

        IFDEF TEST_SYM
        LD BC,TEST_SYM  ; Should be assembled
        ENDIF
        
        IFDEF UNDEFINED_SYM
        LD DE,5678H     ; Should be skipped
        ENDIF
        
; Test IFNDEF  
        IFNDEF UNDEFINED_SYM
        LD HL,9ABCH     ; Should be assembled
        ENDIF
        
        IFNDEF TEST_SYM
        LD IX,0DEADH    ; Should be skipped
        ENDIF
        
; Test IF1 (only in pass 1)
        IF1
        ; This would only be assembled in pass 1
        ; But we're in pass 1 during first pass
        ENDIF
        
; Test IF2 (only in pass 2)
        IF2
        ; This would only be assembled in pass 2
        ENDIF
        
        RET
        END

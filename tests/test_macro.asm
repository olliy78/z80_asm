; Test file for MACRO functionality
; Tests basic macro definition and expansion

        NAME    MACTEST
        
        ORG     0100H

; Simple macro without parameters
BANNER  MACRO
        DB      '*************'
        ENDM

; Macro with parameters
PUTC    MACRO   char
        LD      A, '&char'
        CALL    CONOUT
        ENDM

; Macro with multiple parameters
MOVE16  MACRO   src, dst
        LD      HL, &src
        LD      DE, &dst
        LD      BC, 16
        LDIR
        ENDM

; Macro with LOCAL labels
SKIP0   MACRO   reg
        LOCAL   CONT
        LD      A, &reg
        OR      A
        JR      NZ, CONT
        LD      A, 1
CONT:   RET
        ENDM

; Test macro invocations
START:
        BANNER                  ; Expand BANNER macro
        
        PUTC    H               ; Expand PUTC with 'H'
        PUTC    I               ; Expand PUTC with 'I'
        
        MOVE16  SOURCE, DEST    ; Expand MOVE16
        
        SKIP0   B               ; First SKIP0 - generates ??0001
        SKIP0   C               ; Second SKIP0 - generates ??0002
        
        RET

CONOUT  EQU     0005H
SOURCE: DS      16
DEST:   DS      16

        END

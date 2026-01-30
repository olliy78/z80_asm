; Test program demonstrating both PUBLIC and EXTRN in one module
; This shows a module that both exports and imports symbols

        .Z80
        
        NAME    MIXED
        
; Import external symbols
        EXTRN   system_init, system_halt
        
; Export our symbols
        PUBLIC  module_entry, module_data
        
        CSEG
        
; Exported entry point
module_entry:
        ; Call external initialization
        CALL    system_init
        
        ; Do our work
        LD      HL, module_data
        LD      A, (HL)
        INC     A
        LD      (HL), A
        
        ; Call external cleanup
        CALL    system_halt
        RET

; Private function (not exported)
private_helper:
        XOR     A
        RET

        DSEG

; Exported data
module_data:
        DB      0

        END

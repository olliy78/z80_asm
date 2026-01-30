; Test program for interrupt and special Z80 instructions

        ORG     0000h
        
; Interrupt Mode 0 setup
        IM      0
        DI                      ; Disable interrupts
        
; Initialize interrupt vector
        LD      A, 0C3h         ; JP opcode
        LD      (0038h), A      ; Mode 1 interrupt vector
        LD      HL, int_handler
        LD      (0039h), HL
        
; Switch to IM 1
        IM      1
        EI                      ; Enable interrupts
        
; Wait for interrupt
main_loop:
        NOP
        HALT
        JR      main_loop

; Interrupt handler
int_handler:
        PUSH    AF
        PUSH    HL
        
        ; Do interrupt work
        LD      HL, int_count
        INC     (HL)
        
        POP     HL
        POP     AF
        RETI                    ; Return from interrupt

; Non-maskable interrupt handler (at 0066h)
        ORG     0066h
nmi_handler:
        PUSH    AF
        
        ; Handle NMI
        LD      A, 0FFh
        OUT     (80h), A
        
        POP     AF
        RETN                    ; Return from NMI

; Main program continued
        ORG     0100h
        
; Test IM 2 (interrupt mode 2)
test_im2:
        DI
        IM      2
        LD      A, 80h          ; High byte of interrupt vector table
        LD      I, A            ; Set interrupt page register
        EI
        
; Test I and R register operations
test_ir_regs:
        LD      A, 12h
        LD      I, A            ; Load interrupt register
        LD      A, I            ; Read back
        
        LD      A, 34h
        LD      R, A            ; Load refresh register
        LD      A, R            ; Read back (will be different due to refresh)
        
; Test RLD/RRD (BCD digit rotation)
test_rld_rrd:
        LD      HL, bcd_data
        LD      A, 12h
        LD      (HL), 34h
        
        RLD                     ; Rotate left digit
        ; A = 13h, (HL) = 42h
        
        RRD                     ; Rotate right digit
        ; A = 12h, (HL) = 34h
        
        RET

; DJNZ loop example
delay_loop:
        LD      B, 255
delay:
        DJNZ    delay           ; Decrement B and loop if not zero
        RET

; Data
int_count:
        DB      0
bcd_data:
        DB      0

        END

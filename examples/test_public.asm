; Test program for PUBLIC directive
; This module exports symbols that can be used by other modules

        .Z80
        
        NAME    MODTEST
        
; Public symbols - exported for use by other modules
        PUBLIC  init_device, read_status, write_data
        PUBLIC  device_base
        
; Code segment
        CSEG
        
; Exported initialization routine
init_device:
        LD      A, 0FFh
        OUT     (device_base), A
        RET

; Exported status read routine
read_status:
        IN      A, (device_base)
        AND     80h             ; Check busy bit
        RET

; Exported data write routine  
write_data:
        ; Expects data in A
        OUT     (device_base+1), A
        RET

; Internal (private) helper routine
internal_delay:
        LD      B, 255
delay_loop:
        DJNZ    delay_loop
        RET

; Data segment
        DSEG

; Exported device base address
device_base:
        DB      80h

        END

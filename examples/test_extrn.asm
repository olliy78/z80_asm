; Test program for EXTRN directive
; This module imports symbols from another module

        .Z80
        
        NAME    MAINPROG
        
; External symbols - imported from other modules
        EXTRN   init_device, read_status, write_data
        EXTRN   device_base
        
; Also test alternative syntax
        EXT     helper_func
        
; Code segment
        CSEG
        
; Main program using external functions
main:
        ; Initialize the device
        CALL    init_device
        
        ; Wait for device ready
wait_ready:
        CALL    read_status
        JR      NZ, wait_ready
        
        ; Write some data
        LD      A, 42h
        CALL    write_data
        
        ; Use external data
        LD      A, (device_base)
        
        ; Call external helper
        CALL    helper_func
        
        ; Done
        HALT

; Local function
local_func:
        LD      A, 1
        RET

        END     main

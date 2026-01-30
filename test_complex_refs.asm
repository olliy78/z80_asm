; Complex test with forward/backward references
    NAME    COMPLEX
    ORG     0100H
    
; Entry point
START:
    LD      SP,STACK_TOP    ; Forward ref to STACK_TOP
    CALL    INIT            ; Forward ref to INIT
    JP      MAIN_LOOP       ; Forward ref to MAIN_LOOP
    
; Initialization routine  
INIT:
    LD      A,0
    LD      B,10
    RET
    
; Main loop
MAIN_LOOP:
    CALL    PROCESS         ; Forward ref to PROCESS
    DJNZ    MAIN_LOOP       ; Backward ref (relative)
    RET
    
; Process routine
PROCESS:
    INC     A
    CP      100
    JR      Z,DONE          ; Forward ref to DONE (relative)
    RET
    
DONE:
    LD      A,0FFH
    RET
    
; Data area
DATA_AREA:
    DB      0,1,2,3,4
    DW      1234H
    
; Stack (reserve 100 bytes)
    DS      100
STACK_TOP:
    
    END     START

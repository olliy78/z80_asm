; Test forward reference
    NAME    FWDREF
    ORG     0100H
    
START:
    JR      LOOP        ; Forward reference to LOOP
    NOP
    
LOOP:
    DEC     A
    JR      NZ,LOOP     ; Backward reference
    RET
    
    END     START

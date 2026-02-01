; Simple test listing generation
    NAME    TESTLIST
    ORG     0100H
    
START:
    NOP
    LD      A,10
    LD      B,20H
    LD      HL,1234H
    ADD     A,B
    RET
    
DATA:
    DB      0,1,2,3,4
    DW      1234H,5678H
    
    END     START

; Test listing generation
    NAME    TESTLIST
    
START:
    LD      A,10
    LD      B,A
    LD      HL,0100H
    LD      (HL),A
    
LOOP:
    DEC     B
    JR      NZ,LOOP
    
    RET
    
DATA:
    DB      0,1,2,3,4
    DW      1234H
    
    END     START

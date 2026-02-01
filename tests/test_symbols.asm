; Test listing with symbols
    NAME    TESTSYM
    ORG     0100H
    
MAXVAL  EQU     0FFH
BUFSIZE EQU     256
PORT_A  EQU     00H
    
START:
    LD      A,MAXVAL
    LD      BC,BUFSIZE
    OUT     (PORT_A),A
    RET
    
    END     START

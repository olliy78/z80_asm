    ORG     100H
    LD      A,0FFH
    BIT     0,A
    SET     7,A
    RES     3,B
    RET

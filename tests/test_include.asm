; Test INCLUDE directive
        ORG 1000H
        
        INCLUDE test_include_header.asm
        
        LD BC,HEADER_CONST
        LD DE,HEADER_VALUE
        
; Test ASET directive
counter ASET 0
        LD A,counter
counter ASET counter + 1
        LD B,counter
counter ASET counter + 10
        LD C,counter
        
        RET
        END

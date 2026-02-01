    .z80

hexdb   MACRO   hv
        db      '&hv','H'
        ENDM

hexout  MACRO   hv
        .RADIX  16
        hexdb   %(hv)
        .RADIX  10
        ENDM

value   equ     1234h
        hexout  value
        ret
        END

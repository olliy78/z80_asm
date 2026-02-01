    .z80

value   equ     1234h

hexdb   MACRO   hv
        db      '&hv','H'
        ENDM

hexout  MACRO   hv
        .RADIX  16
        hexdb   %(hv)
        .RADIX  10
        ENDM

        hexout  value
        ret
        END

    .z80

hexdb   MACRO   hv
        db      '&hv','H'
        ENDM

        hexdb   1234
        ret
        END

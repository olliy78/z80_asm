    .z80

hexdb   MACRO   hv
        db      '&hv','H'
        ENDM

hexout  MACRO   hv
        hexdb   %(hv)
        ENDM

        hexout  1234h
        ret
        END

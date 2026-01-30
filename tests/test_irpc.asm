; Test IRPC directive
; Iterate over characters in a string

        NAME    IRPCTEST
        ORG     0100H

; Use IRPC to create character table
ALPHABET:
        IRPC    char, ABCDEFGH
        DB      '&char'
        ENDM

; Use IRPC to generate bit masks
BITMASKS:
        IRPC    bit, 01234567
        DB      1 SHL &bit
        ENDM

; Use IRPC in code
PRINTSTR:
        IRPC    ch, HELLO
        LD      A, '&ch'
        CALL    CONOUT
        ENDM
        RET

CONOUT  EQU     0005H

        END

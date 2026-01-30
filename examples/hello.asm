;
; Simple CP/M "Hello World" program
; Tests basic Z80 instructions and directives
;
        ORG     100H            ; CP/M TPA start

START:  LD      C,9             ; BDOS print string function
        LD      DE,MSG          ; Point to message
        CALL    5               ; Call BDOS
        RET                     ; Return to CCP

MSG:    DB      'Hello, World!$'

        END     START

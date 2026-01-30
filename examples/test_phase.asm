; Test program for .PHASE/.DEPHASE directive
; Demonstrates code assembled at one address but running at another

        ORG     1000h
        
; Code at normal address
start:
        LD      HL, normal_msg
        CALL    print_string
        
        ; Copy phased code to runtime address
        LD      HL, phased_start
        LD      DE, 2000h       ; Runtime address
        LD      BC, phased_end - phased_start
        LDIR
        
        ; Jump to phased code at runtime address
        JP      2000h
        
normal_msg:
        DB      'Normal code', 0

; Code assembled at 1100h but will run at 2000h
phased_start:
        .PHASE  2000h           ; Set phase to runtime address
        
runtime_entry:
        LD      HL, runtime_msg  ; This will use address relative to 2000h
        CALL    runtime_print
        RET
        
runtime_msg:
        DB      'Phased code running!', 0
        
runtime_print:
        ; Print routine
        LD      A, (HL)
        OR      A
        RET     Z
        OUT     (01h), A
        INC     HL
        JR      runtime_print
        
        .DEPHASE                ; End phase block
        
phased_end:

; Back to normal addressing
print_string:
        LD      A, (HL)
        OR      A
        RET     Z
        OUT     (01h), A
        INC     HL
        JR      print_string

        END     start

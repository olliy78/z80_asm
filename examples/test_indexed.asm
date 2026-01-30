; Test program for IX/IY indexed addressing instructions

        ORG 8000h

; Basic IX/IY load operations
        LD IX, 1234h        ; Load IX with immediate
        LD IY, 5678h        ; Load IY with immediate
        LD (9000h), IX      ; Store IX to memory
        LD (9002h), IY      ; Store IY to memory
        LD IX, (9000h)      ; Load IX from memory
        LD IY, (9002h)      ; Load IY from memory

; Stack operations with IX/IY
        PUSH IX             ; Push IX onto stack
        PUSH IY             ; Push IY onto stack
        POP IY              ; Pop into IY
        POP IX              ; Pop into IX

; Arithmetic with IX/IY
        ADD IX, BC          ; Add BC to IX
        ADD IX, DE          ; Add DE to IX
        ADD IX, IX          ; Add IX to IX
        ADD IX, SP          ; Add SP to IX
        ADD IY, BC          ; Add BC to IY
        ADD IY, DE          ; Add DE to IY
        ADD IY, IY          ; Add IY to IY
        ADD IY, SP          ; Add SP to IY

; Increment/Decrement IX/IY
        INC IX              ; Increment IX
        DEC IX              ; Decrement IX
        INC IY              ; Increment IY
        DEC IY              ; Decrement IY

; Indexed memory access - load from (IX+d)
        LD A, (IX+0)        ; Load from IX+0
        LD B, (IX+5)        ; Load from IX+5
        LD C, (IX+10)       ; Load from IX+10
        LD D, (IX-5)        ; Load from IX-5 (negative displacement)
        LD E, (IY+0)        ; Load from IY+0
        LD H, (IY+3)        ; Load from IY+3
        LD L, (IY-2)        ; Load from IY-2

; Indexed memory access - store to (IX+d)
        LD (IX+0), A        ; Store A to IX+0
        LD (IX+1), B        ; Store B to IX+1
        LD (IX+2), C        ; Store C to IX+2
        LD (IX+3), D        ; Store D to IX+3
        LD (IX+4), E        ; Store E to IX+4
        LD (IX+5), H        ; Store H to IX+5
        LD (IX+6), L        ; Store L to IX+6
        LD (IY+0), A        ; Store A to IY+0

; Immediate load to indexed memory
        LD (IX+10), 42h     ; Store immediate to (IX+10)
        LD (IY+20), 0FFh    ; Store immediate to (IY+20)

; Increment/Decrement indexed memory
        INC (IX+5)          ; Increment byte at (IX+5)
        DEC (IX+5)          ; Decrement byte at (IX+5)
        INC (IY+3)          ; Increment byte at (IY+3)
        DEC (IY+3)          ; Decrement byte at (IY+3)

; Arithmetic operations with indexed memory
        ADD A, (IX+5)       ; Add (IX+5) to A
        ADC A, (IX+6)       ; Add with carry (IX+6) to A
        SUB (IX+7)          ; Subtract (IX+7) from A
        SBC A, (IX+8)       ; Subtract with carry (IX+8) from A
        AND (IX+9)          ; AND A with (IX+9)
        XOR (IX+10)         ; XOR A with (IX+10)
        OR (IX+11)          ; OR A with (IX+11)
        CP (IX+12)          ; Compare A with (IX+12)

        ADD A, (IY+1)       ; Add (IY+1) to A
        ADC A, (IY+2)       ; Add with carry (IY+2) to A
        SUB (IY+3)          ; Subtract (IY+3) from A
        SBC A, (IY+4)       ; Subtract with carry (IY+4) from A
        AND (IY+5)          ; AND A with (IY+5)
        XOR (IY+6)          ; XOR A with (IY+6)
        OR (IY+7)           ; OR A with (IY+7)
        CP (IY+8)           ; Compare A with (IY+8)

; Special operations
        EX (SP), IX         ; Exchange (SP) with IX
        EX (SP), IY         ; Exchange (SP) with IY
        JP (IX)             ; Jump to address in IX
        JP (IY)             ; Jump to address in IY
        LD SP, IX           ; Load SP from IX
        LD SP, IY           ; Load SP from IY

        END

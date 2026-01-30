; Test advanced Z80 instructions
; Tests: PUSH/POP, EX, rotates/shifts, bit operations

    ORG 0100h

START:
    ; Test PUSH and POP
    LD BC,1234h
    LD DE,5678h
    PUSH BC
    PUSH DE
    POP HL        ; HL = 5678h
    POP BC        ; BC = 1234h
    
    ; Test EX
    LD DE,0ABCDh
    LD HL,0DEF0h
    EX DE,HL      ; Swap DE and HL
    
    ; Test alternate registers
    LD A,42
    EX AF,AF'     ; Swap with shadow AF
    LD A,99
    EX AF,AF'     ; Back to 42
    
    EXX           ; Swap BC,DE,HL with shadow registers
    
    ; Test stack exchange
    LD HL,1111h
    LD SP,8000h
    EX (SP),HL    ; Exchange HL with top of stack
    
    ; Test rotate/shift
    LD A,80h
    RLCA          ; Rotate left circular
    RRCA          ; Rotate right circular
    RLA           ; Rotate left through carry
    RRA           ; Rotate right through carry
    
    ; Test CB-prefixed rotates
    LD B,55h
    RLC B         ; Rotate B left circular
    RRC B         ; Rotate B right circular
    RL B          ; Rotate B left through carry
    RR B          ; Rotate B right through carry
    
    ; Test shifts
    LD C,0AAh
    SLA C         ; Shift left arithmetic
    SRA C         ; Shift right arithmetic
    SRL C         ; Shift right logical
    
    ; Test (HL) rotates with simple HL value
    LD HL,8000h
    RLC (HL)      ; Rotate value at (HL)
    
    ; Test BIT instructions
    LD A,08h      ; bit 3 set
    BIT 3,A       ; Test bit 3
    BIT 0,A       ; Test bit 0
    
    ; Test SET/RES
    LD B,00h
    SET 5,B       ; Set bit 5
    RES 5,B       ; Reset bit 5
    
    ; Test special instructions
    DAA           ; Decimal adjust
    CPL           ; Complement A
    NEG           ; Negate A
    CCF           ; Complement carry
    SCF           ; Set carry
    
    ; Test I/O
    IN A,(10h)    ; Input from port 10h
    OUT (20h),A   ; Output to port 20h
    
    RET

DATA:
    DB 0FFh

    END START

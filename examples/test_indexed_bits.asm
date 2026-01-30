; Test program for indexed bit manipulation instructions (DDCB/FDCB prefix)

        ORG 8000h

; BIT instructions on (IX+d)
        BIT 0, (IX+0)       ; Test bit 0 at IX+0
        BIT 1, (IX+1)       ; Test bit 1 at IX+1
        BIT 2, (IX+5)       ; Test bit 2 at IX+5
        BIT 3, (IX-3)       ; Test bit 3 at IX-3
        BIT 4, (IX+10)      ; Test bit 4 at IX+10
        BIT 5, (IX-5)       ; Test bit 5 at IX-5
        BIT 6, (IX+20)      ; Test bit 6 at IX+20
        BIT 7, (IX+127)     ; Test bit 7 at IX+127

; BIT instructions on (IY+d)
        BIT 0, (IY+0)       ; Test bit 0 at IY+0
        BIT 1, (IY+2)       ; Test bit 1 at IY+2
        BIT 7, (IY-128)     ; Test bit 7 at IY-128

; SET instructions on (IX+d)
        SET 0, (IX+0)       ; Set bit 0 at IX+0
        SET 1, (IX+1)       ; Set bit 1 at IX+1
        SET 2, (IX+5)       ; Set bit 2 at IX+5
        SET 3, (IX-3)       ; Set bit 3 at IX-3
        SET 4, (IX+10)      ; Set bit 4 at IX+10
        SET 5, (IX-5)       ; Set bit 5 at IX-5
        SET 6, (IX+20)      ; Set bit 6 at IX+20
        SET 7, (IX+127)     ; Set bit 7 at IX+127

; SET instructions on (IY+d)
        SET 0, (IY+0)       ; Set bit 0 at IY+0
        SET 7, (IY-128)     ; Set bit 7 at IY-128

; RES instructions on (IX+d)
        RES 0, (IX+0)       ; Reset bit 0 at IX+0
        RES 1, (IX+1)       ; Reset bit 1 at IX+1
        RES 2, (IX+5)       ; Reset bit 2 at IX+5
        RES 3, (IX-3)       ; Reset bit 3 at IX-3
        RES 4, (IX+10)      ; Reset bit 4 at IX+10
        RES 5, (IX-5)       ; Reset bit 5 at IX-5
        RES 6, (IX+20)      ; Reset bit 6 at IX+20
        RES 7, (IX+127)     ; Reset bit 7 at IX+127

; RES instructions on (IY+d)
        RES 0, (IY+0)       ; Reset bit 0 at IY+0
        RES 7, (IY-128)     ; Reset bit 7 at IY-128

; Rotate/Shift instructions on (IX+d)
        RLC (IX+5)          ; Rotate left circular (IX+5)
        RRC (IX+5)          ; Rotate right circular (IX+5)
        RL (IX+5)           ; Rotate left through carry (IX+5)
        RR (IX+5)           ; Rotate right through carry (IX+5)
        SLA (IX+5)          ; Shift left arithmetic (IX+5)
        SRA (IX+5)          ; Shift right arithmetic (IX+5)
        SLL (IX+5)          ; Shift left logical (IX+5) [undocumented]
        SRL (IX+5)          ; Shift right logical (IX+5)

; Rotate/Shift instructions on (IY+d)
        RLC (IY+3)          ; Rotate left circular (IY+3)
        RRC (IY+3)          ; Rotate right circular (IY+3)
        RL (IY+3)           ; Rotate left through carry (IY+3)
        RR (IY+3)           ; Rotate right through carry (IY+3)
        SLA (IY+3)          ; Shift left arithmetic (IY+3)
        SRA (IY+3)          ; Shift right arithmetic (IY+3)
        SLL (IY+3)          ; Shift left logical (IY+3)
        SRL (IY+3)          ; Shift right logical (IY+3)

; Practical example: Bit manipulation in indexed data structure
; Assume IX points to a status flags array
status_check:
        LD IX, status_array ; Point to status array
        BIT 7, (IX+0)       ; Check busy flag
        JR Z, not_busy      ; Jump if not busy
        RES 7, (IX+0)       ; Clear busy flag
not_busy:
        SET 0, (IX+0)       ; Set ready flag
        
        ; Rotate byte at offset
        RLC (IX+5)          ; Rotate configuration byte
        
        END

status_array:   DB 0FFh, 00h, 55h, 0AAh, 0F0h, 0Fh

        END

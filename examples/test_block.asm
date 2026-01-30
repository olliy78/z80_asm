; Test program for block transfer, compare and I/O instructions

        ORG 8000h

; Block transfer instructions (non-repeating)
        LDI                 ; Load and increment: (DE) <- (HL), DE++, HL++, BC--
        LDD                 ; Load and decrement: (DE) <- (HL), DE--, HL--, BC--

; Block transfer instructions (repeating)
        LDIR                ; Repeat LDI until BC=0
        LDDR                ; Repeat LDD until BC=0

; Block compare instructions (non-repeating)
        CPI                 ; Compare and increment: Compare A with (HL), HL++, BC--
        CPD                 ; Compare and decrement: Compare A with (HL), HL--, BC--

; Block compare instructions (repeating)
        CPIR                ; Repeat CPI until BC=0 or match found
        CPDR                ; Repeat CPD until BC=0 or match found

; Block input instructions (non-repeating)
        INI                 ; Input and increment: (HL) <- (C), HL++, B--
        IND                 ; Input and decrement: (HL) <- (C), HL--, B--

; Block input instructions (repeating)
        INIR                ; Repeat INI until B=0
        INDR                ; Repeat IND until B=0

; Block output instructions (non-repeating)
        OUTI                ; Output and increment: (C) <- (HL), HL++, B--
        OUTD                ; Output and decrement: (C) <- (HL), HL--, B--

; Block output instructions (repeating)
        OTIR                ; Repeat OUTI until B=0
        OTDR                ; Repeat OUTD until B=0

; Practical example: Copy memory block
copy_data:
        LD HL, source       ; Source address
        LD DE, dest         ; Destination address
        LD BC, 100          ; Number of bytes
        LDIR                ; Copy block

; Practical example: Search for byte in memory
search_byte:
        LD HL, buffer       ; Buffer address
        LD BC, 256          ; Buffer size
        LD A, 'X'           ; Byte to find
        CPIR                ; Search for byte

; Practical example: Read block from port
read_port:
        LD HL, input_buffer ; Input buffer
        LD B, 64            ; Number of bytes
        LD C, 10h           ; Port number
        INIR                ; Read block from port

; Practical example: Write block to port
write_port:
        LD HL, output_data  ; Output data
        LD B, 64            ; Number of bytes
        LD C, 20h           ; Port number
        OTIR                ; Write block to port

source:         DB 'Test data', 0
dest:           DS 100
buffer:         DS 256
input_buffer:   DS 64
output_data:    DB 'Output', 0

        END

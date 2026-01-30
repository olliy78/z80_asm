; Test program for extended I/O instructions (IN r,(C) and OUT (C),r)

        ORG 8000h

; Extended input instructions - IN r,(C)
; Read from port specified in C register into various registers
        LD C, 10h           ; Set port number to 10h
        IN A, (C)           ; Input from port (C) to A
        IN B, (C)           ; Input from port (C) to B
        IN C, (C)           ; Input from port (C) to C
        IN D, (C)           ; Input from port (C) to D
        IN E, (C)           ; Input from port (C) to E
        IN H, (C)           ; Input from port (C) to H
        IN L, (C)           ; Input from port (C) to L

; Extended output instructions - OUT (C),r
; Write from various registers to port specified in C register
        LD C, 20h           ; Set port number to 20h
        OUT (C), A          ; Output A to port (C)
        OUT (C), B          ; Output B to port (C)
        OUT (C), C          ; Output C to port (C)
        OUT (C), D          ; Output D to port (C)
        OUT (C), E          ; Output E to port (C)
        OUT (C), H          ; Output H to port (C)
        OUT (C), L          ; Output L to port (C)

; Practical example: Read status register
read_status:
        LD C, 80h           ; Status port
        IN A, (C)           ; Read status
        BIT 0, A            ; Check ready bit
        JR Z, read_status   ; Wait until ready

; Practical example: Write to control register
write_control:
        LD C, 81h           ; Control port
        LD A, 0Fh           ; Control value
        OUT (C), A          ; Write to control port

; Practical example: Multi-register I/O
multi_io:
        LD BC, 4000h        ; B=40h (data), C=00h (base port)
        OUT (C), B          ; Send data to port 00h
        INC C               ; Next port
        OUT (C), D          ; Send D to port 01h
        INC C               ; Next port
        OUT (C), E          ; Send E to port 02h
        INC C               ; Next port
        IN A, (C)           ; Read response from port 03h

        END

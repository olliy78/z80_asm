; Test string with quote
        org 100h
        
; Simple string
        db 'A'
        
; String with embedded double quote
        db '"'
        
; String with double quote and paren
        db '"('
        
; In an expression (like the bios code)
        db 48 or '0','"('
        
        ret

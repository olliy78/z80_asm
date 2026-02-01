; Test nested macro calls
        org 100h

; Define inner macro
inner   MACRO   param
        db 'Inner: &param'
        ENDM

; Define outer macro that calls inner
outer   MACRO   val
        db 'Outer start'
        inner   &val
        db 'Outer end'
        ENDM

; Call outer macro
        outer   TEST

        ret

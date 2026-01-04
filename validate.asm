; Assembled with 64TASS
;   64tass -c ~.asm -L ~.lst --intel-hex -o ~.hex
; or
;   64tass -c ~.asm -L ~.lst --s-record -o ~.hex 
; Put the above equates into an included file per peripheral or board

            .cpu    "6502"

        .INCLUDE "cpu_symbols.inc"
*   =   $30
TMP    .byte   ?

*   =   $2000
START   
        SEI
        LDX #$FF
        TXS
        CLD                                             
        NOP
        NOP
        NOP
        LDY #125                   ; YR: 1900 = 0 2000 = 100 2026 = 126
        LDX #12                    ; Month
        LDA #25                    ; Day of month
        JSR WEKDAY                 ; Get the day of the Wek    
        BRK

; This routine works for any date from 1900-03-01 to 2155-12-31.
; No range checking is done, so validate input before calling.
;
; I use the formula
;     Weekday = (day + offset[month] + year + year/4 + fudge) mod 7
; where the value of fudge depends on the century.
;
; Input: Y = year (0=1900, 1=1901, ..., 255=2155)
;        X = month (1=Jan, 2=Feb, ..., 12=Dec)
;        A = day (1 to 31)
;
; Output: Weekday in A (0=Sunday, 1=Monday, ..., 6=Saturday)

WEKDAY:
        CLD             ; RDA: original code has a bug.  Will not work if D flag set on entry!
        CPX #3          ; Year starts in March to bypass
        BCS MARCH       ; leap year problem
        DEY             ; If Jan or Feb, decrement year
MARCH   EOR #$7F        ; Invert A so carry works right
        CPY #200        ; Carry will be 1 if 22nd century
        ADC MTAB-1,X    ; A is now day+month offset
        STA TMP
        TYA             ; Get the year
        JSR MOD7        ; Do a modulo to prevent overflow
        SBC TMP         ; Combine with day+month
        STA TMP
        TYA             ; Get the year again
        LSR             ; Divide it by 4
        LSR
        CLC             ; Add it to y+m+d and fall through
        ADC TMP
MOD7    ADC #7          ; Returns (A+3) modulo 7
        BCC MOD7        ; for A in 0..255
        RTS
MTAB     .TEXT            1,5,6,3,1,5,3,0,4,2,6,4       ; Month offsets

        

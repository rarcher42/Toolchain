
; Assembled with 64TASS
; 	64tass -c binmon.asm -L binmon.lst --intel-hex -o binmon.hex; 
; Put the above equates into an included file per peripheral or board

        	.cpu    "65816"

		.INCLUDE "via_symbols.inc"



MASK0		= %00000001
MASK1		= %00000010
MASK2		= %00000100
MASK3		= %00001000
MASK4		= %00010000
MASK5		= %00100000
MASK6		= %01000000
MASK7		= %10000000

; Flag definition to OR for SEP, REP
N_FLAG		= MASK7
V_FLAG		= MASK6
M_FLAG		= MASK5
X_FLAG		= MASK4
D_FLAG		= MASK3
I_FLAG		= MASK2
Z_FLAG		= MASK1
C_FLAG		= MASK0

; How to compute the day of the week in 6502 assembly.
; By Paul Guertin (pg@sff.net), 18 August 2000.

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
	* 	= $06
TMP		.byte	?


	*	= $2000
	
RAMSTART
		SEP	#(M_FLAG | X_FLAG)		
		SEC
		XCE				; And enter emulation mode!
		
		LDY	#125		; 2025
		LDX	#12			; December
		LDA	#25			; 25th (a Thursday)
		JSR	WEKDAY		; Get the day of the Wek!
		BRK				; and check the results
HERE	BRA	HERE

WEKDAY:
         CPX #3          ; Year starts in March to bypass
         BCS MARCH       ; leap year problem
         DEY             ; If Jan or Feb, decrement year
MARCH    EOR #$7F        ; Invert A so carry works right
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
MOD7     ADC #7          ; Returns (A+3) modulo 7
         BCC MOD7        ; for A in 0..255
         RTS
MTAB     .text 1,5,6,3,1,5,3,0,4,2,6,4   	; Month offsets
			


.end


; Assembled with 64TASS
; 	64tass -c ~.asm -L ~.lst --intel-hex -o ~.hex
; or
;   64tass -c ~.asm -L ~.lst --s-record -o ~.hex 
; Put the above equates into an included file per peripheral or board

        	.cpu    "65816"

		.INCLUDE "cpu_symbols.inc"

*   =   $30
TMP    .byte   ?

* 	= 	$2000
START 		
		SEI
        REP	#(M_FLAG)
        SEP #(X_FLAG)
        CLD
        .al
        .xs
		CLC					        ; Don't/Enter 65c02 emulation mode
		XCE		
        LDA #$6502
        TCD
        LDA #$70FF
        TCS                         ; Set SP=70FF
        LDA #$1234
        LDX #$56                 
        LDY #$9A                
        BRK
        NOP
        BRK


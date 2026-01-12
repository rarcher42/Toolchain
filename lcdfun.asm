#define START_HR	0x12
#define START_MIN	0x00


#define ZH  R31
#define ZL	R30
#define YH	R29
#define YL	R28
#define XH	R27
#define XL	R26

; I/O pins
#define PINA  0x00
#define DDRA  0x01
#define PORTA 0x02

#define PINB  0x03
#define DDRB  0x04
#define PORTB 0x05

#define PINC  0x06
#define DDRC  0x07
#define PORTC 0x08

#define PIND  0x09
#define DDRD  0x0A
#define PORTD 0x0B

#define	M_PRR	0x64
#define SPCR	0x2c
#define	SPSR	0x2d
#define SPDR	0x2e

#define TIMSK1	0x6f

#define TCCR1A	0x80
#define TCCR1B	0x81
#define TCCR1C	0x82

#define TCNT1L	0x84
#define TCNT1H	0x85

#define OCR1AL	0x88
#define OCR1AH	0x89

#define SPIF	0x7



.CSEG
.ORG	$0000
		rjmp	TEST		; 1 interrupt vector #1 - RESET
		nop
		reti				; 2
		nop
		reti				; 3
		nop
		reti				; 4
		nop
		reti				; 5
		nop
		reti				; 6
		nop
		reti				; 7
		nop
		reti				; 8
		nop
		reti				; 9
		nop
		reti				; 10
		nop
		reti 				; 11
		nop
		reti				; 12
		nop
		reti				; 13
		nop
		rjmp TIM1MATCH		; 14
		nop
		reti				; 15
		nop
		reti				; 16
		nop
		reti				; 17
		nop
		reti				; 18
		nop
		reti				; 19
		nop
		reti				; 20
		nop
		reti				; 21
		nop
		reti				; 22
		nop
		reti				; 23
		nop
		reti				; 24
		nop
		reti				; 25
		nop
		reti				; 26
		nop
		reti				; 27
		nop
		reti				; 28
		nop
		reti				; 29
		nop
		reti				; 30
		nop
		reti				; 31


TIM1MATCH:
		RCALL	INCTIME		; Add a second
		RETI

; Delay (R21R20) * 61 uS
DLY61US:
		; Get the 16 bit timer into R18R17
		LDS		R17,TCNT1L
		LDS		R18,TCNT1H
		ADD		R20,R17
		ADC		R21,R18
DLYWAI:	LDS		R17,TCNT1L
		LDS		R18,TCNT1H
		CP		R17,R20
		BRNE	DLYWAI
		CP		R18,R21
		BRNE	DLYWAI		
		RET

; CMD in R16
LCDCMD:	PUSH	R0
		MOV		R0,R16
		; Output high nibble of command
		MOV		R16,R0			; Get cmd 
		SWAP	R16				; get high order byte in low order bits first
		ANDI	R16,$0F			; discard upper part of byte
		ORI		R16,$40			; turn on E bit (bit 6) to disable write
		OUT		PORTA,R16		; output to LCD display
		LDI		R21,$00
		LDI		R20,$01
		RCALL	DLY61US			; delay
		MOV		R16,R0
		SWAP	R16
		ANDI	R16,$0F
		OUT		PORTA,R16
		LDI		R21,$00
		LDI		R20,$05
		RCALL	DLY61US			; delay
		; Now output the low nibble of command		  
		MOV		R16,R0			; Get cmd 
		ANDI	R16,$0F			; discard upper part of byte
		ORI		R16,$40			; turn on E bit (bit 6) to disable write
		OUT		PORTA,R16		; output to LCD display
		LDI		R21,$00
		LDI		R20,$01
		RCALL	DLY61US			; delay
		MOV		R16,R0
		ANDI	R16,$0F
		OUT		PORTA,R16
		LDI		R21,$00
		LDI		R20,100
		RCALL	DLY61US			; delay
		POP		R0
		RET

; R16 = character position(0..79)
; pos = 00..19 = line #0 ; col/characters = 0..19
; pos = 20..39 = line #1 ; col/characters = 0..19
; pos = 40..59 = line #2 ; col/characters = 0..19
; pos = 60..79 = line #3 ; col/characters = 0..19
; Caller is responsible for ensuring a valid value 0..79 is passed in R16
;
MOVETO:	STS		POS,R16			; store the new character position in the memory 
	 	CPI		R16,60
		BRCC	MT6079
		CPI		R16,40
		BRCC	MT4059
		CPI		R16,20
		BRCC	MT2039
MT0019:	; Is the first line, row#0, pos is 00..19
		ORI		R16,$80			; or in character position 0..19 with $80
		RJMP	LCDCMD			; LCDCMD($80 | col = 0..19) and back to our caller (both end in RET!)
MT2039:	; Is the second line, row#1, pos is 20..39
		SUBI	R16,20
		ORI		R16,$C0
		RJMP	LCDCMD			; LCDCMD($C0 | col = 0..19)
MT4059:	; Is the third line, row #2, pos is 40..59
		SUBI	R16,40
		ORI		R16,$94
		RJMP	LCDCMD			; LCDCMD($94 | col = 0..19)
MT6079:	; Is the fourth line, row #3, pos is 60..79
		SUBI	R16,60
		ORI		R16,$D4			
		RCALL	LCDCMD			; LCDCMD($D4 | col = 0..19)
		RET

; DATA in R16
LCDDATA:
		MOV		R0,R16			; save data in R0
		LDS		R16,POS			; get character position
LCDT20:	CPI		R16,20			; handle possible new line situation
		BRNE	LCDT40
		LDI		R16,$C0
		RCALL	LCDCMD
		RJMP	LCDNONL
LCDT40:	CPI		R16,40
		BRNE	LCDT60
		LDI		R16,$94
		RCALL	LCDCMD
		RJMP	LCDNONL
LCDT60:	CPI		R16,60
		BRNE	LCDNONL
		LDI		R16,$D4
		RCALL	LCDCMD
LCDNONL:
		; Output high nibble of command
		MOV		R16,R0			; Get cmd 
		SWAP	R16				; get high order byte in low order bits first
		ANDI	R16,$0F			; discard upper part of byte
		ORI		R16,$50			; turn on E & RS bits (bits 4 & 6) to disable write
		OUT		PORTA,R16		; output to LCD display
		LDI		R21,$00
		LDI		R20,$01
		RCALL	DLY61US			; delay
		MOV		R16,R0
		SWAP	R16
		ANDI	R16,$0F
		ORI		R16,$10			; set RS, activate E/ to clock in the data
		OUT		PORTA,R16
		LDI		R21,$00
		LDI		R20,$05
		RCALL	DLY61US			; delay
		; Now output the low nibble of command		  
		MOV		R16,R0			; Get cmd 
		ANDI	R16,$0F			; discard upper part of byte
		ORI		R16,$50			; turn on E & RS bits (bits 4 & 6) to disable write
		OUT		PORTA,R16		; output to LCD display
		LDI		R21,$00
		LDI		R20,$01
		RCALL	DLY61US			; delay
		MOV		R16,R0
		ANDI	R16,$0F
		ORI		R16,$10
		OUT		PORTA,R16
		LDI		R21,$00
		LDI		R20,100
		RCALL	DLY61US			; delay
		; Now advance the cursor position by one
		LDS		R16,POS			; get the cursor position
		INC		R16			; 
		CPI		R16,80
		BRCS	LCDDC2			; borrow = good, means POS = 0..79	
		CLR		R16				; No borrow, >= 80: back to character position #0
LCDDC2:	STS		POS,R16			; store the new cursor position in POS
		RET

; Clear the LCD screen
LCDCLS:
		LDI		R16,$01
		RCALL	LCDCMD
		; Set cursor position to home
		LDI		R16,$0
		STS		POS,R16
		RET		

INITLCD:
		LDI		R16,$FF
		OUT		DDRA,R16		; LCD PORTA all outputs!
		LDI		R16,$FF
		OUT		PORTA,R16		; RW E disabled
		LDI		R21,$01
		LDI		R20,$2c
		RCALL	DLY61US
		LDI		R16,$33
		RCALL	LCDCMD
		LDI		R21,$00
		LDI		R20,100
		RCALL	DLY61US
		LDI		R16,$32
		RCALL	LCDCMD
		LDI		R16,$28
		RCALL	LCDCMD
		LDI		R16,$0c
		RCALL	LCDCMD
		RCALL	LCDCLS
		RET

; Output an LCD string 
LCDSTR:

TEST:	CLI					; Disable interrupts
		LDI		R16,$FF
		OUT		DDRA,R16		; LCD PORTA all outputs!
		// RCALL	INITTMR
		// SEI
TESTC0:	// RCALL	INITLCD
	
TESTC1:	
		LDI		R16,$00
		OUT		PORTA,R16
		LDI		R21,$40
		LDI		R20,$01
		RCALL	DLY61US
		// LDI		R16,$02
		LDI		R16,$FF
		OUT		PORTA,R16
		LDI		R21,$40
		LDI		R20,$01
		RCALL	DLY61US
		RJMP	TESTC1	


; Set up the I/O bits as follows:
; PORTA drives dp, segments a-g of LED 
; PORTB bits 2, 1, and 0 select LED digit 0-7, where 0 is leftmost digit
INITLED:
		LDI		R16,$0			; 
		LDI		R17,$FF			; All port pins output
		OUT		PORTA,R16		; All LED segments low (off)
		OUT		DDRA,R17		; LED segments a-g,dp are output
		LDI		R16,$00			; Select no digit
		LDI		R17,$bf			; Only MISO is an input on PORT B
		OUT		PORTB,R16		; Select no digit output
		OUT		DDRB,R17		; set direction
		CLR		R16
		STS		NEXTDIG,R16		; start with first digit to display
		LDI		R16,$0			; 12:00 ?
		MOV		R10,R16
		LDI		R16,$0
		MOV		R11,R16
		RET

INITKEYS:
		LDI		R16,$00			; port D is inputs, PD7 = HR++, PD6 = MIN++, PD5 = SEC<-- 00 (SYNC)
		OUT		DDRD,R16
		STS		DBCNT,R16		; no consecutive agreed values
								; assume, to start, that all keys are down (deliberately wrong!)
		STS		DBVAL,R16		; This guarantees that we store actual reading on first call to CHKKEYS
		STS		KEYDOWN,R16		; no key is down (yet) we hope
		RET

; Initialize the 16 bit timer
INITTMR:
		LDI		R16,$FF			; Reset on match to $4000. Note: write high byte then low, read in opposite order
		STS		OCR1AH,R16			
		LDI		R16,$FF
		STS		OCR1AL,R16
		LDI		R16,$00
		STS 	TCCR1A,R16
		LDI		R16,$0c			; divide by 256 and reset on OCR1A count for 1 Hz reset rate
		STS		TCCR1B,R16
		LDI		R16,$00
		STS		TCCR1C,R16
		LDI		R16,$2			; Enable OCR1A match interrupt only
		STS		TIMSK1,R16
		LDI		R16,START_HR
		STS		HRS,R16
		LDI		R16,START_MIN
		STS		MINS,R16
		RET	
		
		
TMRFILL:
		; Get the 16 bit timer into R18R17
		LDS		R17,TCNT1L
		LDS		R18,TCNT1H
DISPHEX16:							; Display 16 bit hex value in R18R17
		MOV		R16,R18				; Get the high byte of the timer
		SWAP	R16
		ANDI	R16,$0F
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF,R0
		MOV		R16,R18				; Get high byte again
		ANDI	R16,$0F				; low nibble of high byte
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF+1,R0
		;
		MOV		R16,R17				; Get low byte
		SWAP	R16
		ANDI	R16,$0F
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF+2,R0
		MOV		R16,R17				; Get low byte
		ANDI	R16,$0F				; low nibble of low byte
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF+3,R0
		RET
		
							; 
CHKKEYS:
		IN		R16,PIND		; Get input pins on port D
		MOV		R3,R16			; save in R3 for later use (don't want pin changes during routine)
		LDS		R17,DBVAL		; last value read
		CP		R16,R17
		BRNE	CHKNEQ			; not equal -- reset consecutive count
		LDS		R16,DBCNT		; get the consecutive count
		INC		R16
		BREQ	CHKKX1			; if it's $FF, don't increment and falsely get a zero count
		STS		DBCNT,R16		; Store the new consecutive count
		CPI		R16,15
		BRNE	CHKKX1			; keep on debouncing until exactly n times the same (n is checked against R16)
		; If key was down, and is now up 5 consecutive times, then release key and quit, else register key
		LDS		R18,KEYDOWN		; key down?
		BREQ	NEWKY			; no, so it is now!
		;Yes, so just clear keydown to re-debounce
		LDI		R18,$00
		STS		NEWKY,R18
		STS 	DBCNT,R18
		STS 	DBVAL,R3		
		RJMP	CHKKX1
		; Register keypress
NEWKY:	LDI		R16,$ff			; key is now officially down
		STS		KEYDOWN,R16
		MOV		R17,R3			; get key pattern in R17
		ANDI	R17,$E0			; only look at PD7 PD6 and PD5
		CPI		R17,$60			; check for PD7
		BRNE	CHKKC1
		RCALL	INCHRS			; HRS++
		RJMP	CHKKX1
CHKKC1:	CPI		R17,$A0			; check for PD6
		BRNE	CHKKC2
		RCALL	INCMINS			; MINS++
		RJMP	CHKKX1
CHKKC2:	CPI		R17,$C0			; check for PD5  	
		BRNE	CHKKX1			; done checking
		LDI		R16,0
		STS		SECS,R16		; SECS <== 0  (sync clock)
		RJMP	CHKKX1
CHKNEQ:	
		STS		DBVAL,R16		; store the new value for comparison next time
		LDI		R16,0
		STS		DBCNT,R16		; No consecutive equal counts debounced yet
CHKKX1:	RET



		
LEDFILL:
		LDS		R16,HRS			; get BCD hours
		SWAP	R16
		ANDI	R16,$0F
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF,R0
		LDS		R16,HRS
		ANDI	R16,$0F
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF+1,R0
		;
		LDS		R16,MINS			; get BCD minutes
		SWAP	R16
		ANDI	R16,$0F
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF+2,R0
		LDS		R16,MINS
		ANDI	R16,$0F
		MOV		R0,R16
		RCALL	LEDLKUP
		STS		DISPBUF+3,R0
		RET

; Subroutine to add a second to the time
INCTIME:
INCSECS:
		LDS		R16,SECS
		INC		R16				; BCD seconds
		STS		SECS,R16
		ANDI	R16,$0f
		CPI		R16,10
		BRCC	INCSECS			
		LDS		R16,SECS
		CPI		R16,$60
		BRNE	FXIT1
		CLR		R16
		STS		SECS,R16
		RJMP	INCMINS

; Add a minute to the time
INCMINS:
		LDS		R16,MINS
		INC		R16				; BCD minutes
		STS		MINS,R16
		ANDI	R16,$0f
		CPI		R16,10
		BRCC	INCMINS			
		LDS		R16,MINS
		CPI		R16,$60
		BRNE	FXIT1
		CLR		R16
		STS		MINS,R16
		; Next hour due to minute overflow
INCHRS:
		LDS		R16,HRS
		INC		R16				; BCD hours
		STS		HRS,R16
		ANDI	R16,$0f
		CPI		R16,10
		BRCC	INCHRS			
		LDS		R16,HRS
		CPI		R16,$24
		BRNE	FXIT1
		CLR		R16
		STS		HRS,R16
FXIT1:	RET



; Call with R0 being the character code to output to LED
; and R1 addressing the segment we wish to output to (0-7)
LEDPAT:	LDI		ZH,HIGH(2*LEDTBL)	; Byte address = 2 * (word address)
		LDI		ZL,LOW(2*LEDTBL)	;			"
		ADD		ZL,R0				;
		BRCC	LPTC1				; continue unless carry-out to ZH
		INC		ZH					; handle carry into ZH
LPTC1:	LPM							; Read segment patterns from code table
LEDRAW: 
		;COM		R0					; 0 means "on" whereas table is based on 1=on
		LDI		R16,$7
		AND		R1,R16				; mask off any extra bits
		OUT		PORTB,R1			; select digit
		OUT		PORTA,R0			; output the pattern
		RET	


; Call w/ character code in R0.  Returns with segment code (not yet inverted) in R0
LEDLKUP:
		LDI		ZH,HIGH(2*LEDTBL)	; Byte address = 2 * (word address)
		LDI		ZL,LOW(2*LEDTBL)	;			"
		ADD		ZL,R0				;
		BRCC	LPKC1				; continue unless carry-out to ZH
		INC		ZH					; handle carry into ZH
LPKC1:	LPM							; Read segment patterns from code table
		RET	
		

; Display the pattern stored in DISPBUF in the 4 digit 7 segment display.
; DISPBUF[0] is leftmost digit; DISPBUF[3] is rightmost.  Except only
; update one digit at a time per call, so interrupt routine can "pace" the
; scanning and avoid wait-for delays inside an interrupt routine.
LEDSCAN:
		LDI		R16,$00					; No common anode display selected since we're changing pattern
		OUT		PORTB,R16				;	This allows us to change pattern without visible 'glitching'
		LDI		ZL,LOW(DISPBUF)
		LDI		ZH,HIGH(DISPBUF)
		LDS		R1,NEXTDIG
		; Select the addressed digit - let's just do 0-3 for now
		MOV		R16,R1
		CPI		R16,0
		BRNE	LSC7
		LDI		R16,$08
		RJMP	LSCA
LSC7:	CPI		R16,1
		BRNE	LSC8
		LDI		R16,$04
		RJMP	LSCA
LSC8:	CPI		R16,2
		BRNE	LSC9
		LDI		R16,$02
		RJMP	LSCA
LSC9:	LDI		R16,$01
LSCA:	MOV		R2,R16						; save digit select code in R2
		LDI		R16,$3
		AND		R1,R16						; Limit to 0-3
		ADD		ZL,R1
		BRCC	LSC1	
		INC		ZH
LSC1:   LD		R0,Z					; Get the addressed digit
		LDI		R16,$00					; No common anode display selected since we're changing pattern
		OUT		PORTB,R16				;	This allows us to change pattern without visible 'glitching'
		; Time to output digit R16 pattern R0
		OUT		PORTA,R0	; Output the segment pattern
		OUT		PORTB,R2				; output the digit select signal
		; Next time we call, display the next digit in the sequence
		INC		R1
		LDI		R16,$4
		CP		R1,R16
		BRNE	LEDSX1
		CLR		R1				; Wrap-around to digit #0 
LEDSX1:	STS		NEXTDIG,R1
		RET

DELAY:	
DLY0:	LDI		R20,1
DLY1:	LDI		R19,$80
DLY2:	DEC		R19
		BRNE	DLY2
DLY3:	DEC		R20
		BRNE	DLY1
		DEC		R21
		BRNE	DLY0
DLYX1:	RET


MEMTEST:
		LDI		R16,$0			; Next byte value to write out
		MOV		ZH,R16
		MOV		ZL,R16
MWLP:	LDI		R16,$65			; compute next byte to write
		RCALL	SPIWR			; write to SPI memory
		ADIW	ZL,1			; next location
		CPI		ZH,$80			; end of memory?
		BRNE	MWLP			; nope, keep on going
; Now verify that each location from 0 through $7FFF holds correct
		LDI		R16,$0			; 
		MOV		ZH,R16
		MOV		ZL,R16
VFYLP:	LDI		R16,$0
		RCALL	SPIRD			; read the value
		CPI		R16,$65			; compare
		BRNE	MEMERR			; if not equal, CRASH (for now)
		ADIW	ZL,1			; next location
		CPI		ZH,$80
		BRNE	VFYLP			; keep verifying
		; If we got here, we must have matched all locations
		RET						
MEMERR:	RJMP	MEMERR			; crash for now

INITSPI:
		; Make sure SPI port is powered up: PRSPI <-- 0
		LDS		R16,M_PRR
		ANDI	R16,$fb		; Clear bit #2 (PRSPI)
		STS		M_PRR,R16
		; Set MOSI and SCK output, PB3 = /CS for 23K256 SPI SRAM; PB2-PB0 select LED digit, /SS=output
		LDI 	R16,$bf
		OUT		DDRB,R16
		; Enable SPI, Master, set clock rate fck/4
		LDI		R16,$50
		OUT 	SPCR,R16
		LDI		R16,1
		OUT		SPSR,R16		; set SPI2X, clock rate is now fclk/2
		; Configure for sequential mode
		CBI		PORTB,3		; Chip select the SRAM
		LDI		R16,$01		; write config (WRSR command)
		RCALL	SPIRTX
		LDI		R16,$41
		RCALL	SPIRTX		; sequential mode; ignore HOLD pin
		SBI		PORTB,3		; Release chip-select
		RET

; Write the value in R16 into SPI RAM location Z	
SPIWR:
		CBI		PORTB,3		; Chip select the RAM
		PUSH	R16			; save value to write
		LDI		R16,2		; write command
		RCALL	SPIRTX
		MOV		R16,ZH		; high address bits
		RCALL	SPIRTX
		MOV		R16,ZL		; low address bits
		RCALL	SPIRTX
		POP		R16			; value to write out
		RCALL	SPIRTX
		SBI		PORTB,3		; release /CS to RAM
		RET

; Read the value at SPI RAM location Z into R16	
SPIRD:
		CBI		PORTB,3		; Chip select the RAM
		LDI		R16,3		; read command
		RCALL	SPIRTX
		MOV		R16,ZH		; high address bits
		RCALL	SPIRTX
		MOV		R16,ZL		; low address bits
		RCALL	SPIRTX
		LDI		R16,$FF		; just clock out whatever to clock in data into R16
		RCALL	SPIRTX
		SBI		PORTB,3		; release /CS to RAM
		RET

; Call once to configure a block read starting at location Z.  Return nothing
SPISTART:
		CBI		PORTB,3		; Chip select the RAM
		LDI		R16,3		; read command
		RCALL	SPIRTX
		MOV		R16,ZH		; high address bits
		RCALL	SPIRTX
		MOV		R16,ZL		; low address bits
		RCALL	SPIRTX
		RET
		
; Read the 1st through n-th byte in our transfer as needed
SPIRNEXT:
		LDI		R16,$FF		; clock out whatever and clock in data from RAM
		RCALL	SPIRTX
		RET			

; End the multiple-byte RAM read sequence
SPIREND:
		SBI		PORTB,3		; end the transfer
		RET


; Transmit R16 over SPI.  Put received byte in R16 and return	
SPIRTX:
		; Start transmission of data (r16)
		OUT 	SPDR,R16
SPIWX1:
		IN		R17,SPSR
		ANDI	R17,$80
		BREQ	SPIWX1
		IN		R16,SPDR
		RET



LEDTBL:		.DB	$3f,$06,$5b,$4f,$66,$6d,$7d,$07	; '0' - '7'
            .DB $7f,$6f,$77,$7c,$58,$5e,$79,$71	; '8' - 'F'
			.DB	$6f,$74,$04,$1e,$70,$38,$d4,$54	; 'g' - 'n'
			.DB	$5c,$73,$e7,$50,$ed,$78,$1c,$1d ; 'o' - 'v'
			.DB	$9c,$64,$6e,$1b,$86,$d2,$61,$49	; 'w' - misc
			.DB	$64,$1d,$58,$4c,$01,$02,$04,$08
			.DB	$10,$20,$40,$80,$81,$82,$84,$88
			.DB	$90,$A0,$C0,$F0,$00,$00,$00,$00

.DSEG
.ORG 	$0100


POS:	.BYTE		1		; Character position of the LCD

HRS:
		.BYTE		1
MINS:
		.BYTE		1
SECS:
		.BYTE		1

DISPBUF:
		.BYTE		4		; Four digit display output segment values
NEXTDIG:
		.BYTE		1		; Which digit to light up next
DBCNT:
		.BYTE		1		; debounce count
DBVAL:
		.BYTE		1		; Port D (input) values for PD7, PD6, PD5
KEYDOWN:
		.BYTE		1		; non-zero if key is down


; Assembled with 64TASS
; 	64tass -c binmon.asm -L binmon.lst --intel-hex -o binmon.hex; 
; Put the above equates into an included file per peripheral or board

        	.cpu    "65816"

		.INCLUDE "via_symbols.inc"


BS		= $08
LF		= $0A
CR		= $0D
DLE		= $10
CTRL_P  = DLE
SP		= $20
DEL		= $7F
; Frame format characters
ST_X	= $02
CTRL_B	= ST_X
ETX		= $03
CTRL_C	= ETX
SOF		= ST_X
EOF		= ETX
ACK		= $06
NAK		= $15

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
M_FLAG		= MASK6
X_FLAG		= MASK4
D_FLAG		= MASK3
I_FLAG		= MASK2
Z_FLAG		= MASK1
C_FLAG		= MASK0

;
*		= $40
CMD_STATE	
		.byte	?				; CMD_PROC state
CMD_ERROR	
		.byte	?				; Flag error 
CMD_IX		
		.word	?				; Nexxt char in CMD_BUF  @ *(CMD_BUF + CMD_IX)

EA_L	.byte	?				; 24 bit read pointer LOW
EA_H	.byte	?				; 	" HIGH
EA_B	.byte	?				; 	" PAGE #
EA_PTR	=	EA_L				; Address of EA_PTR

CNT_L	.byte	?				; Must be 16 bits to transfer 16 bit index register
CNT_H	.byte	?
CNT		= CNT_L

; Breakpoint logic
BK_ADL	.byte	?
BK_ADH	.byte	?
BK_B	.byte	?
BK_PTR	= BK_ADL				; Where is the breakpoint
BK_L	.byte	?				; Replaced low byte original contents
BK_H	.byte	?				; Replaced high byte original contents

TEMP	.byte	?

SIZE_CMD_BUF	= 512			; maximum command length
*		= $0400					; CMD buffer
CMD_BUF		
		.fill	SIZE_CMD_BUF


* = $2000
		.xl
		.as
RAMSTART	
		SEP	#M_FLAG
		REP #(X_FLAG | D_FLAG)
		LDY	#DUM_MSG
		JSR	PUTSY				; Break the code
		LDA	#$12
		XBA
		LDA	#$34
		LDX	#$5678
		LDY	#$9ABC
HERE	BRK
		BRA	HERE				

DUM_MSG:
		.text	CR,LF
		.text 	"A very dumb message",CR,LF,0

FIFO_TXE 	= PB0
FIFO_RXF	= PB1
FIFO_WR 	= PB2
FIFO_RD 	= PB3
FIFO_PWREN 	= PB5
FIFO_DEBUG 	= PB7					; Handy debug toggle 

; Non-blocking Put FIFO.  Return with carry flag set if buffer is full and nothing was output. 
; Return carry clear upon successful queuing.  Save input char so it doesn't need to be reloaded should FIFO be full
PUT_FRAW	
		PHA							; save output character
		LDA	SYSTEM_VIA_IORB			; Read in FIFO status Port for FIFO
		AND	#FIFO_TXE				; If TXE is low, we can accept data into FIFO.  If high, return immmediately
		CLC							; FIFO is full, so don't try to queue it!	Signal failure
		BNE	OFX1					; 0 = OK to write to FIFO; 1 = Wait, FIFO full!
			; FIFO has room - write A to FIFO in a series of steps
OFCONT		
		STZ	SYSTEM_VIA_DDRA			; (Defensive) Start with Port A input/floating 
		LDA	#(FIFO_RD + FIFO_WR)	; RD=1 WR=1 (WR must go 1->0 for FIFO write)
		STA	SYSTEM_VIA_IORB			; Make sure write is high (and read too!)
		PLA							; Restore the data to send
		PHA							; Also save for exit restore
		STA	SYSTEM_VIA_IORA			; Set up output value in advance in Port A (still input so doesn't go out yet) 
		LDA	#$FF					; make Port A all outputs with stable output value already set in prior lines
		STA	SYSTEM_VIA_DDRA			; Save data to output latches
		NOP							; Some settling time of data output just to be safe
		; Now the data's stable on PA0-7, pull WR line low (leave RD high)
		LDA	#(FIFO_RD)				; RD=1 WR=0 (WR1->0 transition triggers FIFO transfer!)
		STA	SYSTEM_VIA_IORB			; Low-going WR pulse should latch data
		NOP							; Hold time following write strobe, to ensure value is latched OK
		STZ	SYSTEM_VIA_DDRA			; Make port A an input again
		SEC							; signal success of write to caller
OFX1		
		PLA							; restore input character, N and Z flags
		RTS
		
PUTCH	; Blocking char output
		JSR	PUT_FRAW
		BCC	PUTCH
		RTS
	.xl
	.as
	
; Point Y at your NULL-TERMNATED data string
PUTSY 		
		LDA	0,Y
		BEQ	PUTSY1				; Don't print the NULL terminator 
PUTSXL1		
		JSR	PUTCH
		INY						; Prepare to get next character
		BRA	PUTSY
PUTSY1 		
		RTS 
;
;

.end					; finally.  das Ende.  Fini.  It's over.  Go home!


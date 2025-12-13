
; Assembled with 64TASS
; 	64tass -c binmon.asm -L binmon.lst --intel-hex -o binmon.hex; 
; Put the above equates into an included file per peripheral or board

        	.cpu    "65816"

		.INCLUDE "cpu_symbols.inc"
		.INCLUDE "via_symbols.inc"

; Memory Map
;
; 00:8000 - 00:FFFF		-	32K Mapped flash ROM from 128K flash
; 00:7F00 - 00:7FFF		-	I/O space (various peripherals)
; 00:7E00 - 00:7EFF		- 	Monitor direct PAGE
; 00:7000 - 00:7DFF		-	Monitor stack area
; 00:0000 - 00:6FFF		-	Development area*

BS		= 	$08
LF		= 	$0A
CR		= 	$0D
DLE		= 	$10
CTRL_P  = 	DLE
SP		= 	$20
DEL		= 	$7F
; Frame format characters
ST_X	= 	$02
CTRL_B	= 	ST_X
ETX		= 	$03
CTRL_C	= 	ETX
SOF		= 	ST_X
EOF		= 	ETX
ACK		= 	$06
NAK		= 	$15

* 		= 	$40					; Zero page assignments
;

;
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
CNT		= 	CNT_L

TEMP	.byte	?


SIZE_CMD_BUF	= 	768			; maximum command length - pathological worst case for 256 payload bytes+
*		= 	$0400					; CMD buffer
CMD_BUF		
		.fill	SIZE_CMD_BUF

* 		= $7E00
; Mirror the user state upon BRK exit state
; Note: do not re-order the next 16 bytes
M_EMODE	
		.byte	?				; Track the mode: (formerly called M_EFLAG but changed
								; to multi-state to remember M and X flags)
								; $FF = Emulation mode (65c816 = 65c02 more-or-less)
								; $00 = Native mode, M=0 X=0 16 bit A,M, 16 bit X,Y
								; $01 = Native mode, M=0 X=1 16 bit A,M, 8 bits X,Y
								; $02 = Native mode, M=1 X=0 8 bits A,M, 16 bits X,Y
								; $03 = Native mode, M=1 X=1 8 bits A,M, 8 bits X,Y
M_STATE	=	M_EMODE 
M_FLAGS 				
		.byte	?				; 8 bits
M_A		.byte	?				; lower 8 bits of A
M_B		.byte	?				; B/upper 8 bits of A
M_X		.word	?				; Always save as 16 bits
M_Y		.word 	?				; Always save as 16 bits
M_SP	.word	?				; Always save as 16 bits
M_DPR	.word	?			; Always 16 bits
M_PC	.word	?
M_PBR	.byte	?			; Always 8 bits
M_DBR	.byte   ?                       ; Always 8 bits




STACKTOP	= 	$7CFF				; Top of RAM (I/O 0x7F00-0x7FFF)



* 	= 	$F800
		.xl
		.as
START 		
		SEI
		CLC					; Enter native 65c816 mode
		XCE					; 
		REP	#(X_FLAG | D_FLAG)		; 16 bit index, binary mode
		.xl
		SEP	#M_FLAG				; 8 bit A (process byte stream)
		.as
		LDX	#STACKTOP			; Set 16bit SP to usable RAMtop
		TXS						; Set up the stack pointer
		JSR	INIT_FIFO			; initialize FIFO
		LDY	#VER_MSG
		JSR	PUTSY
CMD_INIT 	
		JSR	INIT_CMD_PROC			; Prepare processor state machine
CMD_LOOP
		JSR	CMD_PROC			; Run processor state machine
		BRA	CMD_LOOP			; then do it some more

VER_MSG
		.text	CR,LF
		.text  	"************************",CR,LF
		.text	"*     BinMon v0.1      *",CR,LF
		.text	"*     Ross Archer      *",CR,LF
		.text	"*   MIT License Use    *",CR,LF
		.text	"*   27 November 2025   *",CR,LF
		.text	"************************",CR,LF
		.text	0
; END main monitor program

		
CMD_TBL 	
		.word	CMD_STATE_INIT
		.word	CMD_STATE_AWAIT_SOF
		.word 	CMD_STATE_COLLECT
		.word	CMD_STATE_TRANSLATE
		.word	CMD_STATE_PROCESS
		.word	CMD_STATE_INIT
; Subroutines begin here


INIT_CMD_PROC	
		STZ	CMD_STATE			; Make sure we start in INIT state
		LDX	#0
		STX	CMD_IX				; Must be w/in bounds before INIT state
		RTS

; Run the command processing FSM
CMD_PROC 	
		; Bounds check the command buffer and discard if overflow would occur
		LDX	CMD_IX	
		CPX	#SIZE_CMD_BUF
		BCC	CMD_PC1
		STZ	CMD_STATE			; discard as command can't be valid
CMD_PC1 	
		; Jump to the current state
		LDA	#0
		XBA					; B = 0
		LDA	CMD_STATE		; get state
		ASL	A				; two bytes per entry
		TAX					; 16 bit table offset (B|A)->X
        JMP	(CMD_TBL,X)		; execute the current state
		; No RTS - that happens in each finite state 


; State 0: INIT
CMD_STATE_INIT  
		STZ	CMD_ERROR			; no command error (yet)
		LDX	#0					; start at beginning of CMD_BUF
		STX	CMD_IX				; store 16 bit pointerkk
		LDA	#1
		STA	CMD_STATE
		RTS
		
; State 1: AWAIT_SOF
CMD_STATE_AWAIT_SOF
		JSR	GET_FRAW
		BCC	CMD_AX1				; Nothing waiting
		CMP	#SOF
		BNE	CMD_AX1
		LDA	#2
		STA	CMD_STATE	
CMD_AX1 
		RTS

; State 2: COLLECT bytes
CMD_STATE_COLLECT
		JSR	GET_FRAW
		BCC	CMD_CX1				; if nothing in FIFO, quit
		CMP	#EOF				; In case we hit the end of text
		BNE	CMD_CC2	
		LDA	#4					; Go process the command
		STA	CMD_STATE
		BRA	CMD_CX1				; 
CMD_CC2	
		CMP	#DLE
		BNE	CMD_CC3
		LDA	#3
		STA	CMD_STATE
		BRA	CMD_CX1
CMD_CC3		
		LDX	CMD_IX
		STA	CMD_BUF,X				; Store in CMD_BUF
		INX							; Increment CMD_IX; point to next free slot
		STX	CMD_IX
CMD_CX1 	
		RTS

; State 3: TRANSLATE DLE'ed sequences
CMD_STATE_TRANSLATE
		JSR	GET_FRAW
		BCC	CMD_TX1				; If nothing in FIFO, quit 
CMD_TC1		
		CMP	#EOF
		BNE	CMD_TC2
		LDA	#4
		STA	CMD_STATE
		BRA	CMD_TX1
CMD_TC2		
		CMP	#$11				; DLE' escaped SOF
		BNE	CMD_TC3
		LDA	#SOF
		BRA	CMD_TXLAT
CMD_TC3		
		CMP	#$12
		BNE	CMD_TC4
		LDA	#DLE
		BRA	CMD_TXLAT
CMD_TC4		
		CMP	#$13
		BNE	CMD_TC5
		LDA	#EOF
		BRA	CMD_TXLAT
CMD_TC5		
		LDA	#1
		STA	CMD_ERROR			; Invalid DLE sequence - flag error
		LDA	#0
CMD_TXLAT	
		LDX	CMD_IX
		STA	CMD_BUF,X		; Store in CMD_BUF
		INX					; Increment CMD_IX
		STX	CMD_IX
		LDA	#2
		STA	CMD_STATE
CMD_TX1 	
		RTS

; State 4: PROCESS the command
CMD_STATE_PROCESS
		STZ	CMD_STATE			; Upon return to FSM loop, we're looking for a new command
		; Parse and dispatch
		LDA	CMD_BUF				; Get the command
		CMP	#1
		BNE	PCBC1
		JMP	RD_CMD
PCBC1		
		CMP	#2
		BNE	PCBC2
		JMP	WR_CMD
PCBC2	
		CMP	#3
		BNE	PCBC3
		JMP	GO_CMD
PCBC3	
		CMP	#4					; set (volatile) Breakpoint
		BNE	PCBC4
		JMP	SET_BP_CMD			; Breakpoint command	
PCBC4	
		CMP	#5					; Get registers
		BNE	PCBC5
		JMP	GET_CONTEXT
PCBC5	
		CMP	#'E'				; echo command
		BNE	PCBERR
		JMP	ECHO_CMD
PCBERR	
		JSR	SEND_NAK			; Unknown cmd
		RTS


; [01][start-address-Low][start-address-high][start-address-page][LEN]		; 
; Read n+1 bytes (1 to 256 inclusive) and Return
; X / CMD_IX is index to next byte
RD_CMD	
		LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
		STA	EA_L
		LDA	CMD_BUF+2
		STA	EA_H
		LDA	CMD_BUF+3
		STA	EA_B
		; Store 8 bit count as 16 bits for indexing
		LDA	CMD_BUF+4
		STA	CNT_L
		STZ	CNT_H
		LDA	#SOF
		JSR	PUTCH			; Unencoded SOF starts frame
		LDY	#$FFFF			; Start at -1 because we send n+1 bytes :D
RD_BN1	
		INY
		LDA	[EA_PTR],Y		; Get next byte
		JSR	CHR_ENCODE		; Send the byte there, possibly DLE escaped as two bytes
		CPY	CNT				; Length word
		BNE	RD_BN1
RD_BX1	
		LDA	#EOF
		JSR	PUTCH			; Unencoded EOF ends frame
		RTS
		
; [02][start-address-Low][start-address-high][start-address-page][b0][b1]...[bn]
; Use CMD_IX to determine last write byte
WR_CMD
		LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
		STA	EA_L
		LDA	CMD_BUF+2
		STA	EA_H
		LDA	CMD_BUF+3
		STA	EA_B
		; Store 8 bit count as 16 bits for indexing
		LDA	CMD_IX
		STA	CNT_L
		LDA	CMD_IX+1
		STA	CNT_H
		LDX	#4				; Index of first CMD_BUF byte to write
		LDY	#0				; Where to write
WR_BN1	
		LDA	CMD_BUF,X		; Get the next buffer byte
		STA	[EA_PTR],Y		; Write it out
		INX
		INY
		CPX	CNT
		BNE	WR_BN1
		JSR	SEND_ACK
		RTS
	

; [03][start-address-low][start-address-high][start-address-high]	
GO_CMD	
		LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
		STA	EA_L
		LDA	CMD_BUF+2
		STA	EA_H
		LDA	CMD_BUF+3
		STA	EA_B
		JSR	SEND_ACK
		JML [EA_PTR]
		
; [04][start-address-low][start-address-high][start-address-high]
; returns: replaced byte value (caller is responsible for replacing it!)	
SET_BP_CMD	
		LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
		STA	EA_L
		LDA	CMD_BUF+2
		STA	EA_H
		LDA	CMD_BUF+3
		STA	EA_B
		LDA	#SOF
		JSR	PUTCH
		LDA	[EA_PTR]
		; SEND a character inter-packet (translating)
		JSR	CHR_ENCODE
		LDA	#EOF
		JSR	PUTCH
		LDA	#$00			; BRK instruction (can't STZ indirect long)
		STA [EA_PTR]		; Save a BRK there
		RTS

; [05] GETCONTEXT
; Returns saved registers in block order 
; Returns [E-flag][Flags][A][B][XL][XH][YL][YH][SPL][SPH][DPRL][DPRH][PCL][PCH][PBR][DBR]
GET_CONTEXT
		LDA	#SOF
		JSR	PUTCH
		LDX	#0
SSNC1
		LDA	M_STATE,X
		JSR	CHR_ENCODE
		INX
		CPX	#16
		BNE	SSNC1
		LDA	#EOF
		JSR	PUTCH
		RTS
		
; ['E'][data]
; replies [data]
ECHO_CMD
		LDA	#SOF
		JSR	PUTCH
		LDX	#0
EC_CM1	
		LDA	CMD_BUF,X
		JSR	CHR_ENCODE		; Send byte, possibly DLE'ing it for transmission
		INX
		CPX	CMD_IX
		BNE	EC_CM1
		LDA	#EOF
		JSR	PUTCH
		RTS


;;;; ============================= New FIFO functions ======================================
; Initializes the system VIA (the USB debugger), and syncs with the USB chip.

FIFO_TXE 	= 	PB0
FIFO_RXF	= 	PB1
FIFO_WR 	= 	PB2
FIFO_RD 	= 	PB3
FIFO_PWREN 	= 	PB5
FIFO_DEBUG 	= 	PB7					; Handy debug toggle 


INIT_FIFO
		LDA	#$FF
		STA SYSTEM_VIA_PCR			; CB2=FAMS=flash A16=1;  CA2=FA15=A15=1; Select flash Bank #3
		STZ SYSTEM_VIA_ACR			; Disable PB7, shift register, timer T1 interrupt.  Not absolutely required while interrupts are disabled FIXME: set up timer
		STZ	SYSTEM_VIA_DDRA			; Set PA0-PA7 to all inputs
		STZ	SYSTEM_VIA_DDRB			; In case we're not coming off a reset, make PORT B an input and change output register when it's NOT outputting
		LDA	#FIFO_RD				;
		STA	SYSTEM_VIA_IORB			; Avoid possible glitch by writing to output latch while Port B is still an input (after reset)
		LDA	#(FIFO_RD + FIFO_WR)		; Make FIFO RD & WR pins outputs so we can strobe data in and out of the FIFO
		STA	SYSTEM_VIA_DDRB			; Port B: PB2 and PB3 are outputs; rest are inputs from earlier IORB write
		RTS					

		
; On exit:
; If Carry flag is set, A contains the next byte from the FIFO
; If carry flag is clear, no character was received and A doesn't contain anything meaningful
GET_FRAW
		LDA	SYSTEM_VIA_IORB			; Check RXF flag
		AND	#FIFO_RXF				; If clear, we're OK to read.  If set, there's no data waiting
		CLC							; Assume no character (overridden if A != 0)
		BNE INFXIT					; If RXF is 1, then no character is waiting!
		STZ	SYSTEM_VIA_DDRA			; Make Port A inputs
		LDA	#FIFO_RD
		STA	SYSTEM_VIA_IORB			; RD=1 WR=0 (RD must go to 0 to read
		NOP
		STZ	SYSTEM_VIA_IORB			; RD=0 WR=0	- FIFO presents data to port A	
		NOP
		LDA	SYSTEM_VIA_IORA			; read data in
		PHA
		LDA	#FIFO_RD				; Restore back to inactive signals RD=1 and WR=0
		STA	SYSTEM_VIA_IORB
		PLA
		SEC							; we got a byte!
INFXIT	
		RTS


	
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
		


; SEND NAK packet
SEND_NAK
		LDA	#NAK
		STA	TEMP
		BRA	SENDC1
; SEND ACK packet
SEND_ACK
		LDA	#ACK
		STA	TEMP
SENDC1	
		LDA	#SOF
		JSR	PUTCH
		LDA	TEMP
		JSR	PUTCH
		LDA	#EOF
		BRA	PUTCH
		
; This subroutine translates SOF DLE and EOF for inside-packet protection of OOB characters.  Enter at PUTCH by itself for untranslated output	
CHR_ENCODE
		CMP	#SOF
		BNE	WENC1
		LDA	#DLE
		JSR	PUTCH
		LDA	#$11
		BRA	PUTCH
WENC1	
		CMP	#DLE
		BNE	WENC2
		LDA	#DLE
		JSR	PUTCH
		LDA	#$12
		BRA	PUTCH
WENC2	
		CMP	#EOF
		BNE	PUTCH
		LDA	#DLE
		JSR	PUTCH
		LDA	#$13
PUTCH	
		; Blocking char output
		JSR	PUT_FRAW
		BCC	PUTCH
		RTS
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

; Native BREAK exception handler:  Save full machine context 
; and return to monitor with machine context.
; Also, remember M and X flag settings and native mode 
NAT_SAV_CONTEXT
		SEP	#M_FLAG			; 
		.as
		STA	M_A
		XBA
		STA	M_B
		PLA
		STA	M_FLAGS			; Pull flags put on stack by BRK instruction
		AND #$30			; Figure which E mode
		LSR	A
		LSR	A
		LSR	A
		LSR	A
		STA	M_EMODE			; Remember the flag combinations		
BNICP1
		REP	#X_FLAG			; 16 bit index, binary mode
		.xl
		STX	M_X
		STY	M_Y
		PLX					; Pull PC15..0 return address off stack
		DEX					; points past BRK... restore to point to BRK for continue
		DEX					; " Now we're pointing at BRK.  Handler should restore the byte here
		STX	M_PC			; save PC=*(BRK instruction)
		; End restore instruction byte
		PLA					; Get PBR of code
		STA	M_PBR
		TSX					; Now we're pulled everything off stack - it's pre-BRK position
		STX	M_SP
		PHD					; save DPR (zero page pointer)
		PLX					
		STX	M_DPR
		PHB					; Save 
		PLA
		STA	M_DBR
		; Fake up the stack to return to system monitor
BNRET		
		LDA	#0				; Monitor is in bank #0
		PHA					; push PBR=0
		LDX	#START
		PHX					; push "return address"
		PHP					; save flags
		RTI					; Jump to monitor entry

NMI_ISR 	
		RTI

IRQ_ISR 	
		RTI
		
		
		
; Emulated BREAK exception handler:  Came from emulation mode user program!
; return from user code (RAM) to monitor (ROM)
;    
; Note:  M_EMODE captures emulation mode and data size for the 5 mode combinations (0-3 or $FF)
;
;
BRK_IRQ_EMU
		.xs
		.as
		STA	M_A			; Save A and B
		XBA
		STA	M_B
		LDA	#$FF			; We came from emulation mode.  Remember that
		STA	M_EMODE			; E=1; we came from Emulation mode
		PLA				; get flags from stack
		PHA				; and put them back!
		AND	#BRK_FLAG		; Check for BRK flag
		BNE	BRK_SAV_CONTEXT		; continue break handling
		LDA	M_B			; restore A & B 
		XBA
		LDA	M_A
		BRA	IRQ_EMU_ISR		; EMU mode IRQ handler
; Save the entire machine context, remember emulation mode setting, and return to monitor
BRK_SAV_CONTEXT
		PLA				; Get flags off stack 
		STA	M_FLAGS			; Pull flags put on stack by BRK instruction
		STX	M_X
		STZ	M_X+1
		STY	M_Y
		STZ	M_Y+1
		PLA				; Pull PC7..0
		SEC
		SBC	#$02			; Subtract 2 from low address
		STA	M_PC			; store as low PC
		STA EA_L
		PLA				; Get high PC PLA sets N and Z but leaves C alone fortunately
		SBC	#$00			; take care of any borrow from low PC
		STA	M_PC+1			; 
		STA EA_H
		STZ	M_PBR			; Probably garbage, but slightly possibly holding future context 
		TSX					; Now we're pulled everything off stack - it's pre-BRK position
		STX	M_SP
		LDA	#$01
		STA	M_SP+1
		PHD				; save DPR (zero page pointer).  Again, precautionary context save, 
		PLA				; probably not important to user process (unless maybe it switches to native 			
		STA	M_DPR+1			; at some point when it resumes.
		PLA
		STA	M_DPR
		STZ	M_DBR
		STZ	M_PBR
		; Fake up the stack to return to system monitor
		LDA	#>START
		PHA
		LDA	#<START
		PHA
		PHP
		RTI				
; Put IRQ handlers for emulation mode here		
IRQ_EMU_ISR
		.xs
		.as
		RTI
		

; Restore machine to user context
RES_CONTEXT
		.as
		LDA	M_EMODE				; emulation mode?
		BNE	RES_EMU				; If 0, we're restoring to a native context
; Restore machine to user native context
RES_NATIVE
		REP	#(X_FLAG | D_FLAG)
		.xl
		SEP	#(M_FLAG)
		.as
		CLC
		XCE										; go to native mode
		LDX	M_SP								; 16 bit SP
		TXS										; Restore the stack - target stack but we're pointing to free space so OK
		; Set up the stack frame for RTI
		LDA	M_PBR
		PHA
		LDX	M_PC
		PHX										; push 
		LDA	M_FLAGS
		PHA
		; End up setting stack frame for RTI
		LDX	M_DPR
		PHX
		LDA	M_DBR
		PHA
		LDX	M_X
		LDY	M_Y
		LDA	M_B
		XBA
		LDA	M_A
		PLB
		PLD
		RTI										; RTI(24) Must use RTI to restore flags & Full 24 bit PC.  No other way!(?)
; Restore machine to user emulation context
RES_EMU	
		SEP	#(X_FLAG | M_FLAG)
		.xs
		.as
		REP	#(D_FLAG)
		SEC
		XCE					; we're in emulation mode, 8 bit everything
		LDX	M_SP			; only LSByte matters
		TXS					; Now in target stack context (but Ok to add below/free, before returning)
		STZ	M_PBR			; ? Not used AFAIK in emulation mode; should not be restored if not zero?
		LDA	M_DBR
		PHA
		PLB					; Set up the DBR
		LDX	M_X
		LDY	M_Y
		LDA	M_DPR+1
		XBA	
		LDA	M_DPR
		TCD					; Always 16 bits; DPR=B*256+A
		; Set up the RTI frame to restore A, Flags, and PC
		LDA	M_PC+1			; Set up the RTI frame, PCH
		PHA
		LDA	M_PC			; Set up the RTI frame, PCL
		PHA
		LDA	M_FLAGS
		PHA
		; End set up RTI 
		LDA	M_DPR+1
		PHA
		LDA	M_DPR
		PHA
		LDA	M_DBR
		PHA
		; Restore A & B
		LDA   M_B
        XBA
		LDA	  M_A
		PLB
		PLD
		RTI					; RTI(16)


;;; Exception / Reset / Interrupt vectors in native and emulation mode
* 	= 	$FFE4
NCOP	
		.word	START		; COP exception in native mode
* 	= 	$FFE6
NBRK	
		.word	NAT_SAV_CONTEXT	; BRK: Save context (coming from native mode)
* 	= 	$FFE8
NABORT	
		.word	START
* 	= 	$FFEA
NNMI	
		.word	NMI_ISR		; NMI interrupt in native mode
* 	= 	$FFEE
NIRQ	
		.word	IRQ_ISR 	; 

* 	= 	$FFF4
ECOP	
		.word	START		; COP exception in 65c02 emulation mode
* 	= 	$FFF8
EABORT	
		.word	START
* 	= 	$FFFA
ENMI		
		.word	START		; NMI interrupt in 65c02 emulation mode
* 	= 	$FFFC
ERESET	
		.word	START		; RESET exception in all modes
* 	= 	$FFFE
EIRQ	
		.word	BRK_IRQ_EMU		; Note: when enabling IRQ, must test and pick between IRQ and BRK 

.end							; finally.  das Ende.  Fini.  It's over.  Go home!

:20F800007818FBC218E220A2FF7C9A2050FAA01CF820F5FA20C7F820CFF880FB0D0A2A2AF1
:20F820002A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A0D0A2A20202020204269A0
:20F840006E4D6F6E2076302E312020202020202A0D0A2A2020202020526F73732041726343
:20F860006865722020202020202A0D0A2A2020204D4954204C6963656E7365205573652084
:20F880002020202A0D0A2A2020203237204E6F76656D62657220323032352020202A0D0AEC
:20F8A0002A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A2A0D0A00E2F8EEF8FC85
:20F8C000F81EF958F9E2F86440A20000864260A642E0000390026440A900EBA5400AAA7CD6
:20F8E000BBF86441A200008642A9018540602069FA9008C902D004A9028540602069FA90DA
:20F900001CC903D006A90485408012C910D006A90385408008A6429D0004E886426020695B
:20F92000FA9034C903D006A9048540802AC911D004A9028016C912D004A910800EC913D0BA
:20F9400004A9038006A9018541A900A6429D0004E88642A9028540606440AD0004C901D060
:20F96000034C8BF9C902D0034CB9F9C903D0034CE5F9C904D0034CFAF9C905D0034C1DFA6C
:20F98000C945D0034C37FA20B2FA60AD01048544AD02048545AD03048546AD040485476481
:20F9A00048A90220EFFAA0FFFFC8B74420CAFAC447D0F6A90320EFFA60AD01048544AD02F6
:20F9C000048545AD03048546A5428547A5438548A20400A00000BD00049744E8C8E447D0E5
:20F9E000F520B8FA60AD01048544AD02048545AD0304854620B8FADC4400AD01048544AD4F
:20FA000002048545AD03048546A90220EFFAA74420CAFAA90320EFFAA900874460A90220C0
:20FA2000EFFAA20000BD007E20CAFAE8E01000D0F4A90320EFFA60A90220EFFAA20000BD58
:20FA4000000420CAFAE8E442D0F5A90320EFFA60A9FF8DEC7F9CEB7F9CE37F9CE27FA90884
:20FA60008DE07FA90C8DE27F60ADE07F290218D0189CE37FA9088DE07FEA9CE07FEAADE168
:20FA80007F48A9088DE07F68386048ADE07F290118D01D9CE37FA90C8DE07F68488DE17F43
:20FAA000A9FF8DE37FEAA9088DE07FEA9CE37F386860A91585498004A9068549A90220EFF9
:20FAC000FAA54920EFFAA9038025C902D009A91020EFFAA9118018C910D009A91020EFFAB8
:20FAE000A912800BC903D007A91020EFFAA913208AFA90FB60B90000F00620EFFAC880F51B
:20FB000060E2208D027EEB8D037E688D017E29304A4A4A4A8D007EC2108E047E8C067EFA91
:20FB2000CACA8E0C7E688D0E7EBA8E087E0BFA8E0A7E8B688D0F7EA90048A200F8DA0840F5
:20FB400040408D027EEB8D037EA9FF8D007E68482910D009AD037EEBAD027E8044688D01AA
:20FB60007E8E047E9C057E8C067E9C077E6838E9028D0C7E854468E9008D0D7E85459C0E5F
:20FB80007EBA8E087EA9018D097E0B688D0B7E688D0A7E9C0F7E9C0E7EA9F848A900480822
:20FBA0004040AD007ED02EC218E22018FBAE087E9AAD0E7E48AE0C7EDAAD017E48AE0A7EA2
:20FBC000DAAD0F7E48AE047EAC067EAD037EEBAD027EAB2B40E230C20838FBAE087E9A9C94
:20FBE0000E7EAD0F7E48ABAE047EAC067EAD0B7EEBAD0A7E5BAD0D7E48AD0C7E48AD017E10
:17FC000048AD0B7E48AD0A7E48AD0F7E48AD037EEBAD027EAB2B40CC
:08FFE40000F801FB00F840FBEE
:02FFEE0041FBD5
:02FFF40000F813
:08FFF80000F800F800F842FBDC
:00000001FF

; 64tass Turbo Assembler Macro V1.59.3120 listing file
; 64tass -c -L pipeline.lst --intel-hex -o pipeline.hex pipeline.asm
; Fri Dec 12 17:25:58 2025

;Offset	;Hex		;Monitor	;Source

;******  Processing input file: pipeline.asm


;******  Processing file: cpu_symbols.inc

=$01					MASK0		= %00000001
=$02					MASK1		= %00000010
=$04					MASK2		= %00000100
=$08					MASK3		= %00001000
=$10					MASK4		= %00010000
=$20					MASK5		= %00100000
=$40					MASK6		= %01000000
=$80					MASK7		= %10000000
=$80					N_FLAG		= MASK7
=$40					V_FLAG		= MASK6
=$20					M_FLAG		= MASK5
=$10					X_FLAG		= MASK4
=$10					BRK_FLAG	= MASK4				; Emulation mode
=$08					D_FLAG		= MASK3
=$04					I_FLAG		= MASK2
=$02					Z_FLAG		= MASK1
=$01					C_FLAG		= MASK0

;******  Return to file: pipeline.asm


;******  Processing file: via_symbols.inc

=$7fe0					SYS_VIA_BASE	    = 	$7FE0
=32736					SYSTEM_VIA_IORB     =  	SYS_VIA_BASE+0	; Port B IO register
=32737					SYSTEM_VIA_IORA     =	SYS_VIA_BASE+1 	; Port A IO register
=32738					SYSTEM_VIA_DDRB     = 	SYS_VIA_BASE+2	; Port B data direction register
=32739					SYSTEM_VIA_DDRA     = 	SYS_VIA_BASE+3	; Port A data direction register
=32740					SYSTEM_VIA_T1C_L    =	SYS_VIA_BASE+4 	; Timer 1 counter/latches, low-order
=32741					SYSTEM_VIA_T1C_H    = 	SYS_VIA_BASE+5	; Timer 1 high-order counter
=32742					SYSTEM_VIA_T1L_L    = 	SYS_VIA_BASE+6	; Timer 1 low-order latches
=32743					SYSTEM_VIA_T1L_H    = 	SYS_VIA_BASE+7	; Timer 1 high-order latches
=32744					SYSTEM_VIA_T2L    = 	SYS_VIA_BASE+8	; Timer 2 counter/latches, lower-order
=32745					SYSTEM_VIA_T2H    = 	SYS_VIA_BASE+9	; Timer 2 high-order counter
=32746					SYSTEM_VIA_SR       = 	SYS_VIA_BASE+10	; Shift register
=32747					SYSTEM_VIA_ACR      = 	SYS_VIA_BASE+11	; Auxilliary control register
=32748					SYSTEM_VIA_PCR      =	SYS_VIA_BASE+12	; Peripheral control register
=32749					SYSTEM_VIA_IFR	    =	SYS_VIA_BASE+13 ; Interrupt flag register
=32750					SYSTEM_VIA_IER      = 	SYS_VIA_BASE+14	; Interrupt enable register
=32751					SYSTEM_VIA_ORA_IRA  =	SYS_VIA_BASE+15	; Port A IO register, but no handshake
=$7fc0					DEBUG_VIA_BASE	    = 	$7FC0
=32704					DEBUG_VIA_IORB     =  	DEBUG_VIA_BASE+0	; Port B IO register
=32705					DEBUG_VIA_IORA     =	DEBUG_VIA_BASE+1 	; Port A IO register
=32706					DEBUG_VIA_DDRB     = 	DEBUG_VIA_BASE+2	; Port B data direction register
=32707					DEBUG_VIA_DDRA     = 	DEBUG_VIA_BASE+3	; Port A data direction register
=32708					DEBUG_VIA_T1C_L    =	DEBUG_VIA_BASE+4 	; Timer 1 counter/latches, low-order
=32709					DEBUG_VIA_T1C_H    = 	DEBUG_VIA_BASE+5	; Timer 1 high-order counter
=32710					DEBUG_VIA_T1L_L    = 	DEBUG_VIA_BASE+6	; Timer 1 low-order latches
=32711					DEBUG_VIA_T1L_H    = 	DEBUG_VIA_BASE+7	; Timer 1 high-order latches
=32712					DEBUG_VIA_T2C_L    = 	DEBUG_VIA_BASE+8	; Timer 2 counter/latches, lower-order
=32713					DEBUG_VIA_T2C_H    = 	DEBUG_VIA_BASE+9	; Timer 2 high-order counter
=32714					DEBUG_VIA_SR       = 	DEBUG_VIA_BASE+10	; Shift register
=32715					DEBUG_VIA_ACR      = 	DEBUG_VIA_BASE+11	; Auxilliary control register
=32716					DEBUG_VIA_PCR      =	DEBUG_VIA_BASE+12	; Peripheral control register
=32717					DEBUG_VIA_IFR	    =	DEBUG_VIA_BASE+13 ; Interrupt flag register
=32718					DEBUG_VIA_IER      = 	DEBUG_VIA_BASE+14	; Interrupt enable register
=32719					DEBUG_VIA_ORA_IRA  =	DEBUG_VIA_BASE+15	; Port A IO register, but no handshake
=$01					PB0 = MASK0
=$02					PB1 = MASK1
=$04					PB2 = MASK2
=$08					PB3 = MASK3
=$10					PB4 = MASK4
=$20					PB5 = MASK5
=$40					PB6 = MASK6
=$80					PB7 = MASK7
=$01					PA0 = MASK0
=$02					PA1 = MASK1
=$04					PA2 = MASK2
=$08					PA3 = MASK3
=$10					PA4 = MASK4
=$20					PA5 = MASK5
=$40					PA6 = MASK6
=$80					PA7 = MASK7

;******  Return to file: pipeline.asm

=$08					BS		= 	$08
=$0a					LF		= 	$0A
=$0d					CR		= 	$0D
=$10					DLE		= 	$10
=$10					CTRL_P  = 	DLE
=$20					SP		= 	$20
=$7f					DEL		= 	$7F
=$02					ST_X	= 	$02
=$02					CTRL_B	= 	ST_X
=$03					ETX		= 	$03
=$03					CTRL_C	= 	ETX
=$02					SOF		= 	ST_X
=$03					EOF		= 	ETX
=$06					ACK		= 	$06
=$15					NAK		= 	$15
.0040					CMD_STATE
>0040							.byte	?				; CMD_PROC state
.0041					CMD_ERROR
>0041							.byte	?				; Flag error
.0042					CMD_IX
>0042							.word	?				; Nexxt char in CMD_BUF  @ *(CMD_BUF + CMD_IX)
>0044					EA_L	.byte	?				; 24 bit read pointer LOW
>0045					EA_H	.byte	?				; 	" HIGH
>0046					EA_B	.byte	?				; 	" PAGE #
=$44					EA_PTR	=	EA_L				; Address of EA_PTR
>0047					CNT_L	.byte	?				; Must be 16 bits to transfer 16 bit index register
>0048					CNT_H	.byte	?
=$47					CNT		= 	CNT_L
>0049					TEMP	.byte	?
=768					SIZE_CMD_BUF	= 	768			; maximum command length - pathological worst case for 256 payload bytes+
.0400					CMD_BUF
>0400							.fill	SIZE_CMD_BUF
.7e00					M_EMODE
>7e00							.byte	?				; Track the mode: (formerly called M_EFLAG but changed
=$7e00					M_STATE	=	M_EMODE
.7e01					M_FLAGS
>7e01							.byte	?				; 8 bits
>7e02					M_A		.byte	?				; lower 8 bits of A
>7e03					M_B		.byte	?				; B/upper 8 bits of A
>7e04					M_X		.word	?				; Always save as 16 bits
>7e06					M_Y		.word 	?				; Always save as 16 bits
>7e08					M_SP	.word	?				; Always save as 16 bits
>7e0a					M_DPR	.word	?			; Always 16 bits
>7e0c					M_PC	.word	?
>7e0e					M_PBR	.byte	?			; Always 8 bits
>7e0f					M_DBR	.byte   ?                       ; Always 8 bits
=$7cff					STACKTOP	= 	$7CFF				; Top of RAM (I/O 0x7F00-0x7FFF)
.f800					START
.f800	78		sei				SEI
.f801	18		clc				CLC					; Enter native 65c816 mode
.f802	fb		xce				XCE					;
.f803	c2 18		rep #$18			REP	#(X_FLAG | D_FLAG)		; 16 bit index, binary mode
.f805	e2 20		sep #$20			SEP	#M_FLAG				; 8 bit A (process byte stream)
.f807	a2 ff 7c	ldx #$7cff			LDX	#STACKTOP			; Set 16bit SP to usable RAMtop
.f80a	9a		txs				TXS						; Set up the stack pointer
.f80b	20 50 fa	jsr $fa50			JSR	INIT_FIFO			; initialize FIFO
.f80e	a0 1c f8	ldy #$f81c			LDY	#VER_MSG
.f811	20 f5 fa	jsr $faf5			JSR	PUTSY
.f814					CMD_INIT
.f814	20 c7 f8	jsr $f8c7			JSR	INIT_CMD_PROC			; Prepare processor state machine
.f817					CMD_LOOP
.f817	20 cf f8	jsr $f8cf			JSR	CMD_PROC			; Run processor state machine
.f81a	80 fb		bra $f817			BRA	CMD_LOOP			; then do it some more
.f81c					VER_MSG
>f81c	0d 0a						.text	CR,LF
>f81e	2a 2a 2a 2a 2a 2a 2a 2a				.text  	"************************",CR,LF
>f826	2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a
>f836	0d 0a
>f838	2a 20 20 20 20 20 42 69				.text	"*     BinMon v0.1      *",CR,LF
>f840	6e 4d 6f 6e 20 76 30 2e 31 20 20 20 20 20 20 2a
>f850	0d 0a
>f852	2a 20 20 20 20 20 52 6f				.text	"*     Ross Archer      *",CR,LF
>f85a	73 73 20 41 72 63 68 65 72 20 20 20 20 20 20 2a
>f86a	0d 0a
>f86c	2a 20 20 20 4d 49 54 20				.text	"*   MIT License Use    *",CR,LF
>f874	4c 69 63 65 6e 73 65 20 55 73 65 20 20 20 20 2a
>f884	0d 0a
>f886	2a 20 20 20 32 37 20 4e				.text	"*   27 November 2025   *",CR,LF
>f88e	6f 76 65 6d 62 65 72 20 32 30 32 35 20 20 20 2a
>f89e	0d 0a
>f8a0	2a 2a 2a 2a 2a 2a 2a 2a				.text	"************************",CR,LF
>f8a8	2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a 2a
>f8b8	0d 0a
>f8ba	00						.text	0
.f8bb					CMD_TBL
>f8bb	e2 f8						.word	CMD_STATE_INIT
>f8bd	ee f8						.word	CMD_STATE_AWAIT_SOF
>f8bf	fc f8						.word 	CMD_STATE_COLLECT
>f8c1	1e f9						.word	CMD_STATE_TRANSLATE
>f8c3	58 f9						.word	CMD_STATE_PROCESS
>f8c5	e2 f8						.word	CMD_STATE_INIT
.f8c7					INIT_CMD_PROC
.f8c7	64 40		stz $40				STZ	CMD_STATE			; Make sure we start in INIT state
.f8c9	a2 00 00	ldx #$0000			LDX	#0
.f8cc	86 42		stx $42				STX	CMD_IX				; Must be w/in bounds before INIT state
.f8ce	60		rts				RTS
.f8cf					CMD_PROC
.f8cf	a6 42		ldx $42				LDX	CMD_IX
.f8d1	e0 00 03	cpx #$0300			CPX	#SIZE_CMD_BUF
.f8d4	90 02		bcc $f8d8			BCC	CMD_PC1
.f8d6	64 40		stz $40				STZ	CMD_STATE			; discard as command can't be valid
.f8d8					CMD_PC1
.f8d8	a9 00		lda #$00			LDA	#0
.f8da	eb		xba				XBA					; B = 0
.f8db	a5 40		lda $40				LDA	CMD_STATE		; get state
.f8dd	0a		asl a				ASL	A				; two bytes per entry
.f8de	aa		tax				TAX					; 16 bit table offset (B|A)->X
.f8df	7c bb f8	jmp ($f8bb,x)	        JMP	(CMD_TBL,X)		; execute the current state
.f8e2					CMD_STATE_INIT
.f8e2	64 41		stz $41				STZ	CMD_ERROR			; no command error (yet)
.f8e4	a2 00 00	ldx #$0000			LDX	#0					; start at beginning of CMD_BUF
.f8e7	86 42		stx $42				STX	CMD_IX				; store 16 bit pointerkk
.f8e9	a9 01		lda #$01			LDA	#1
.f8eb	85 40		sta $40				STA	CMD_STATE
.f8ed	60		rts				RTS
.f8ee					CMD_STATE_AWAIT_SOF
.f8ee	20 69 fa	jsr $fa69			JSR	GET_FRAW
.f8f1	90 08		bcc $f8fb			BCC	CMD_AX1				; Nothing waiting
.f8f3	c9 02		cmp #$02			CMP	#SOF
.f8f5	d0 04		bne $f8fb			BNE	CMD_AX1
.f8f7	a9 02		lda #$02			LDA	#2
.f8f9	85 40		sta $40				STA	CMD_STATE
.f8fb					CMD_AX1
.f8fb	60		rts				RTS
.f8fc					CMD_STATE_COLLECT
.f8fc	20 69 fa	jsr $fa69			JSR	GET_FRAW
.f8ff	90 1c		bcc $f91d			BCC	CMD_CX1				; if nothing in FIFO, quit
.f901	c9 03		cmp #$03			CMP	#EOF				; In case we hit the end of text
.f903	d0 06		bne $f90b			BNE	CMD_CC2
.f905	a9 04		lda #$04			LDA	#4					; Go process the command
.f907	85 40		sta $40				STA	CMD_STATE
.f909	80 12		bra $f91d			BRA	CMD_CX1				;
.f90b					CMD_CC2
.f90b	c9 10		cmp #$10			CMP	#DLE
.f90d	d0 06		bne $f915			BNE	CMD_CC3
.f90f	a9 03		lda #$03			LDA	#3
.f911	85 40		sta $40				STA	CMD_STATE
.f913	80 08		bra $f91d			BRA	CMD_CX1
.f915					CMD_CC3
.f915	a6 42		ldx $42				LDX	CMD_IX
.f917	9d 00 04	sta $0400,x			STA	CMD_BUF,X				; Store in CMD_BUF
.f91a	e8		inx				INX							; Increment CMD_IX; point to next free slot
.f91b	86 42		stx $42				STX	CMD_IX
.f91d					CMD_CX1
.f91d	60		rts				RTS
.f91e					CMD_STATE_TRANSLATE
.f91e	20 69 fa	jsr $fa69			JSR	GET_FRAW
.f921	90 34		bcc $f957			BCC	CMD_TX1				; If nothing in FIFO, quit
.f923					CMD_TC1
.f923	c9 03		cmp #$03			CMP	#EOF
.f925	d0 06		bne $f92d			BNE	CMD_TC2
.f927	a9 04		lda #$04			LDA	#4
.f929	85 40		sta $40				STA	CMD_STATE
.f92b	80 2a		bra $f957			BRA	CMD_TX1
.f92d					CMD_TC2
.f92d	c9 11		cmp #$11			CMP	#$11				; DLE' escaped SOF
.f92f	d0 04		bne $f935			BNE	CMD_TC3
.f931	a9 02		lda #$02			LDA	#SOF
.f933	80 16		bra $f94b			BRA	CMD_TXLAT
.f935					CMD_TC3
.f935	c9 12		cmp #$12			CMP	#$12
.f937	d0 04		bne $f93d			BNE	CMD_TC4
.f939	a9 10		lda #$10			LDA	#DLE
.f93b	80 0e		bra $f94b			BRA	CMD_TXLAT
.f93d					CMD_TC4
.f93d	c9 13		cmp #$13			CMP	#$13
.f93f	d0 04		bne $f945			BNE	CMD_TC5
.f941	a9 03		lda #$03			LDA	#EOF
.f943	80 06		bra $f94b			BRA	CMD_TXLAT
.f945					CMD_TC5
.f945	a9 01		lda #$01			LDA	#1
.f947	85 41		sta $41				STA	CMD_ERROR			; Invalid DLE sequence - flag error
.f949	a9 00		lda #$00			LDA	#0
.f94b					CMD_TXLAT
.f94b	a6 42		ldx $42				LDX	CMD_IX
.f94d	9d 00 04	sta $0400,x			STA	CMD_BUF,X		; Store in CMD_BUF
.f950	e8		inx				INX					; Increment CMD_IX
.f951	86 42		stx $42				STX	CMD_IX
.f953	a9 02		lda #$02			LDA	#2
.f955	85 40		sta $40				STA	CMD_STATE
.f957					CMD_TX1
.f957	60		rts				RTS
.f958					CMD_STATE_PROCESS
.f958	64 40		stz $40				STZ	CMD_STATE			; Upon return to FSM loop, we're looking for a new command
.f95a	ad 00 04	lda $0400			LDA	CMD_BUF				; Get the command
.f95d	c9 01		cmp #$01			CMP	#1
.f95f	d0 03		bne $f964			BNE	PCBC1
.f961	4c 8b f9	jmp $f98b			JMP	RD_CMD
.f964					PCBC1
.f964	c9 02		cmp #$02			CMP	#2
.f966	d0 03		bne $f96b			BNE	PCBC2
.f968	4c b9 f9	jmp $f9b9			JMP	WR_CMD
.f96b					PCBC2
.f96b	c9 03		cmp #$03			CMP	#3
.f96d	d0 03		bne $f972			BNE	PCBC3
.f96f	4c e5 f9	jmp $f9e5			JMP	GO_CMD
.f972					PCBC3
.f972	c9 04		cmp #$04			CMP	#4					; set (volatile) Breakpoint
.f974	d0 03		bne $f979			BNE	PCBC4
.f976	4c fa f9	jmp $f9fa			JMP	SET_BP_CMD			; Breakpoint command
.f979					PCBC4
.f979	c9 05		cmp #$05			CMP	#5					; Get registers
.f97b	d0 03		bne $f980			BNE	PCBC5
.f97d	4c 1d fa	jmp $fa1d			JMP	GET_CONTEXT
.f980					PCBC5
.f980	c9 45		cmp #$45			CMP	#'E'				; echo command
.f982	d0 03		bne $f987			BNE	PCBERR
.f984	4c 37 fa	jmp $fa37			JMP	ECHO_CMD
.f987					PCBERR
.f987	20 b2 fa	jsr $fab2			JSR	SEND_NAK			; Unknown cmd
.f98a	60		rts				RTS
.f98b					RD_CMD
.f98b	ad 01 04	lda $0401			LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
.f98e	85 44		sta $44				STA	EA_L
.f990	ad 02 04	lda $0402			LDA	CMD_BUF+2
.f993	85 45		sta $45				STA	EA_H
.f995	ad 03 04	lda $0403			LDA	CMD_BUF+3
.f998	85 46		sta $46				STA	EA_B
.f99a	ad 04 04	lda $0404			LDA	CMD_BUF+4
.f99d	85 47		sta $47				STA	CNT_L
.f99f	64 48		stz $48				STZ	CNT_H
.f9a1	a9 02		lda #$02			LDA	#SOF
.f9a3	20 ef fa	jsr $faef			JSR	PUTCH			; Unencoded SOF starts frame
.f9a6	a0 ff ff	ldy #$ffff			LDY	#$FFFF			; Start at -1 because we send n+1 bytes :D
.f9a9					RD_BN1
.f9a9	c8		iny				INY
.f9aa	b7 44		lda [$44],y			LDA	[EA_PTR],Y		; Get next byte
.f9ac	20 ca fa	jsr $faca			JSR	CHR_ENCODE		; Send the byte there, possibly DLE escaped as two bytes
.f9af	c4 47		cpy $47				CPY	CNT				; Length word
.f9b1	d0 f6		bne $f9a9			BNE	RD_BN1
.f9b3					RD_BX1
.f9b3	a9 03		lda #$03			LDA	#EOF
.f9b5	20 ef fa	jsr $faef			JSR	PUTCH			; Unencoded EOF ends frame
.f9b8	60		rts				RTS
.f9b9					WR_CMD
.f9b9	ad 01 04	lda $0401			LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
.f9bc	85 44		sta $44				STA	EA_L
.f9be	ad 02 04	lda $0402			LDA	CMD_BUF+2
.f9c1	85 45		sta $45				STA	EA_H
.f9c3	ad 03 04	lda $0403			LDA	CMD_BUF+3
.f9c6	85 46		sta $46				STA	EA_B
.f9c8	a5 42		lda $42				LDA	CMD_IX
.f9ca	85 47		sta $47				STA	CNT_L
.f9cc	a5 43		lda $43				LDA	CMD_IX+1
.f9ce	85 48		sta $48				STA	CNT_H
.f9d0	a2 04 00	ldx #$0004			LDX	#4				; Index of first CMD_BUF byte to write
.f9d3	a0 00 00	ldy #$0000			LDY	#0				; Where to write
.f9d6					WR_BN1
.f9d6	bd 00 04	lda $0400,x			LDA	CMD_BUF,X		; Get the next buffer byte
.f9d9	97 44		sta [$44],y			STA	[EA_PTR],Y		; Write it out
.f9db	e8		inx				INX
.f9dc	c8		iny				INY
.f9dd	e4 47		cpx $47				CPX	CNT
.f9df	d0 f5		bne $f9d6			BNE	WR_BN1
.f9e1	20 b8 fa	jsr $fab8			JSR	SEND_ACK
.f9e4	60		rts				RTS
.f9e5					GO_CMD
.f9e5	ad 01 04	lda $0401			LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
.f9e8	85 44		sta $44				STA	EA_L
.f9ea	ad 02 04	lda $0402			LDA	CMD_BUF+2
.f9ed	85 45		sta $45				STA	EA_H
.f9ef	ad 03 04	lda $0403			LDA	CMD_BUF+3
.f9f2	85 46		sta $46				STA	EA_B
.f9f4	20 b8 fa	jsr $fab8			JSR	SEND_ACK
.f9f7	dc 44 00	jmp [$0044]			JML [EA_PTR]
.f9fa					SET_BP_CMD
.f9fa	ad 01 04	lda $0401			LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
.f9fd	85 44		sta $44				STA	EA_L
.f9ff	ad 02 04	lda $0402			LDA	CMD_BUF+2
.fa02	85 45		sta $45				STA	EA_H
.fa04	ad 03 04	lda $0403			LDA	CMD_BUF+3
.fa07	85 46		sta $46				STA	EA_B
.fa09	a9 02		lda #$02			LDA	#SOF
.fa0b	20 ef fa	jsr $faef			JSR	PUTCH
.fa0e	a7 44		lda [$44]			LDA	[EA_PTR]
.fa10	20 ca fa	jsr $faca			JSR	CHR_ENCODE
.fa13	a9 03		lda #$03			LDA	#EOF
.fa15	20 ef fa	jsr $faef			JSR	PUTCH
.fa18	a9 00		lda #$00			LDA	#$00			; BRK instruction (can't STZ indirect long)
.fa1a	87 44		sta [$44]			STA [EA_PTR]		; Save a BRK there
.fa1c	60		rts				RTS
.fa1d					GET_CONTEXT
.fa1d	a9 02		lda #$02			LDA	#SOF
.fa1f	20 ef fa	jsr $faef			JSR	PUTCH
.fa22	a2 00 00	ldx #$0000			LDX	#0
.fa25					SSNC1
.fa25	bd 00 7e	lda $7e00,x			LDA	M_STATE,X
.fa28	20 ca fa	jsr $faca			JSR	CHR_ENCODE
.fa2b	e8		inx				INX
.fa2c	e0 10 00	cpx #$0010			CPX	#16
.fa2f	d0 f4		bne $fa25			BNE	SSNC1
.fa31	a9 03		lda #$03			LDA	#EOF
.fa33	20 ef fa	jsr $faef			JSR	PUTCH
.fa36	60		rts				RTS
.fa37					ECHO_CMD
.fa37	a9 02		lda #$02			LDA	#SOF
.fa39	20 ef fa	jsr $faef			JSR	PUTCH
.fa3c	a2 00 00	ldx #$0000			LDX	#0
.fa3f					EC_CM1
.fa3f	bd 00 04	lda $0400,x			LDA	CMD_BUF,X
.fa42	20 ca fa	jsr $faca			JSR	CHR_ENCODE		; Send byte, possibly DLE'ing it for transmission
.fa45	e8		inx				INX
.fa46	e4 42		cpx $42				CPX	CMD_IX
.fa48	d0 f5		bne $fa3f			BNE	EC_CM1
.fa4a	a9 03		lda #$03			LDA	#EOF
.fa4c	20 ef fa	jsr $faef			JSR	PUTCH
.fa4f	60		rts				RTS
=$01					FIFO_TXE 	= 	PB0
=$02					FIFO_RXF	= 	PB1
=$04					FIFO_WR 	= 	PB2
=$08					FIFO_RD 	= 	PB3
=$20					FIFO_PWREN 	= 	PB5
=$80					FIFO_DEBUG 	= 	PB7					; Handy debug toggle
.fa50					INIT_FIFO
.fa50	a9 ff		lda #$ff			LDA	#$FF
.fa52	8d ec 7f	sta $7fec			STA SYSTEM_VIA_PCR			; CB2=FAMS=flash A16=1;  CA2=FA15=A15=1; Select flash Bank #3
.fa55	9c eb 7f	stz $7feb			STZ SYSTEM_VIA_ACR			; Disable PB7, shift register, timer T1 interrupt.  Not absolutely required while interrupts are disabled FIXME: set up timer
.fa58	9c e3 7f	stz $7fe3			STZ	SYSTEM_VIA_DDRA			; Set PA0-PA7 to all inputs
.fa5b	9c e2 7f	stz $7fe2			STZ	SYSTEM_VIA_DDRB			; In case we're not coming off a reset, make PORT B an input and change output register when it's NOT outputting
.fa5e	a9 08		lda #$08			LDA	#FIFO_RD				;
.fa60	8d e0 7f	sta $7fe0			STA	SYSTEM_VIA_IORB			; Avoid possible glitch by writing to output latch while Port B is still an input (after reset)
.fa63	a9 0c		lda #$0c			LDA	#(FIFO_RD + FIFO_WR)		; Make FIFO RD & WR pins outputs so we can strobe data in and out of the FIFO
.fa65	8d e2 7f	sta $7fe2			STA	SYSTEM_VIA_DDRB			; Port B: PB2 and PB3 are outputs; rest are inputs from earlier IORB write
.fa68	60		rts				RTS
.fa69					GET_FRAW
.fa69	ad e0 7f	lda $7fe0			LDA	SYSTEM_VIA_IORB			; Check RXF flag
.fa6c	29 02		and #$02			AND	#FIFO_RXF				; If clear, we're OK to read.  If set, there's no data waiting
.fa6e	18		clc				CLC							; Assume no character (overridden if A != 0)
.fa6f	d0 18		bne $fa89			BNE INFXIT					; If RXF is 1, then no character is waiting!
.fa71	9c e3 7f	stz $7fe3			STZ	SYSTEM_VIA_DDRA			; Make Port A inputs
.fa74	a9 08		lda #$08			LDA	#FIFO_RD
.fa76	8d e0 7f	sta $7fe0			STA	SYSTEM_VIA_IORB			; RD=1 WR=0 (RD must go to 0 to read
.fa79	ea		nop				NOP
.fa7a	9c e0 7f	stz $7fe0			STZ	SYSTEM_VIA_IORB			; RD=0 WR=0	- FIFO presents data to port A
.fa7d	ea		nop				NOP
.fa7e	ad e1 7f	lda $7fe1			LDA	SYSTEM_VIA_IORA			; read data in
.fa81	48		pha				PHA
.fa82	a9 08		lda #$08			LDA	#FIFO_RD				; Restore back to inactive signals RD=1 and WR=0
.fa84	8d e0 7f	sta $7fe0			STA	SYSTEM_VIA_IORB
.fa87	68		pla				PLA
.fa88	38		sec				SEC							; we got a byte!
.fa89					INFXIT
.fa89	60		rts				RTS
.fa8a					PUT_FRAW
.fa8a	48		pha				PHA							; save output character
.fa8b	ad e0 7f	lda $7fe0			LDA	SYSTEM_VIA_IORB			; Read in FIFO status Port for FIFO
.fa8e	29 01		and #$01			AND	#FIFO_TXE				; If TXE is low, we can accept data into FIFO.  If high, return immmediately
.fa90	18		clc				CLC							; FIFO is full, so don't try to queue it!	Signal failure
.fa91	d0 1d		bne $fab0			BNE	OFX1					; 0 = OK to write to FIFO; 1 = Wait, FIFO full!
.fa93					OFCONT
.fa93	9c e3 7f	stz $7fe3			STZ	SYSTEM_VIA_DDRA			; (Defensive) Start with Port A input/floating
.fa96	a9 0c		lda #$0c			LDA	#(FIFO_RD + FIFO_WR)	; RD=1 WR=1 (WR must go 1->0 for FIFO write)
.fa98	8d e0 7f	sta $7fe0			STA	SYSTEM_VIA_IORB			; Make sure write is high (and read too!)
.fa9b	68		pla				PLA							; Restore the data to send
.fa9c	48		pha				PHA							; Also save for exit restore
.fa9d	8d e1 7f	sta $7fe1			STA	SYSTEM_VIA_IORA			; Set up output value in advance in Port A (still input so doesn't go out yet)
.faa0	a9 ff		lda #$ff			LDA	#$FF					; make Port A all outputs with stable output value already set in prior lines
.faa2	8d e3 7f	sta $7fe3			STA	SYSTEM_VIA_DDRA			; Save data to output latches
.faa5	ea		nop				NOP							; Some settling time of data output just to be safe
.faa6	a9 08		lda #$08			LDA	#(FIFO_RD)				; RD=1 WR=0 (WR1->0 transition triggers FIFO transfer!)
.faa8	8d e0 7f	sta $7fe0			STA	SYSTEM_VIA_IORB			; Low-going WR pulse should latch data
.faab	ea		nop				NOP							; Hold time following write strobe, to ensure value is latched OK
.faac	9c e3 7f	stz $7fe3			STZ	SYSTEM_VIA_DDRA			; Make port A an input again
.faaf	38		sec				SEC							; signal success of write to caller
.fab0					OFX1
.fab0	68		pla				PLA							; restore input character, N and Z flags
.fab1	60		rts				RTS
.fab2					SEND_NAK
.fab2	a9 15		lda #$15			LDA	#NAK
.fab4	85 49		sta $49				STA	TEMP
.fab6	80 04		bra $fabc			BRA	SENDC1
.fab8					SEND_ACK
.fab8	a9 06		lda #$06			LDA	#ACK
.faba	85 49		sta $49				STA	TEMP
.fabc					SENDC1
.fabc	a9 02		lda #$02			LDA	#SOF
.fabe	20 ef fa	jsr $faef			JSR	PUTCH
.fac1	a5 49		lda $49				LDA	TEMP
.fac3	20 ef fa	jsr $faef			JSR	PUTCH
.fac6	a9 03		lda #$03			LDA	#EOF
.fac8	80 25		bra $faef			BRA	PUTCH
.faca					CHR_ENCODE
.faca	c9 02		cmp #$02			CMP	#SOF
.facc	d0 09		bne $fad7			BNE	WENC1
.face	a9 10		lda #$10			LDA	#DLE
.fad0	20 ef fa	jsr $faef			JSR	PUTCH
.fad3	a9 11		lda #$11			LDA	#$11
.fad5	80 18		bra $faef			BRA	PUTCH
.fad7					WENC1
.fad7	c9 10		cmp #$10			CMP	#DLE
.fad9	d0 09		bne $fae4			BNE	WENC2
.fadb	a9 10		lda #$10			LDA	#DLE
.fadd	20 ef fa	jsr $faef			JSR	PUTCH
.fae0	a9 12		lda #$12			LDA	#$12
.fae2	80 0b		bra $faef			BRA	PUTCH
.fae4					WENC2
.fae4	c9 03		cmp #$03			CMP	#EOF
.fae6	d0 07		bne $faef			BNE	PUTCH
.fae8	a9 10		lda #$10			LDA	#DLE
.faea	20 ef fa	jsr $faef			JSR	PUTCH
.faed	a9 13		lda #$13			LDA	#$13
.faef					PUTCH
.faef	20 8a fa	jsr $fa8a			JSR	PUT_FRAW
.faf2	90 fb		bcc $faef			BCC	PUTCH
.faf4	60		rts				RTS
.faf5					PUTSY
.faf5	b9 00 00	lda $0000,y			LDA	0,Y
.faf8	f0 06		beq $fb00			BEQ	PUTSY1				; Don't print the NULL terminator
.fafa					PUTSXL1
.fafa	20 ef fa	jsr $faef			JSR	PUTCH
.fafd	c8		iny				INY						; Prepare to get next character
.fafe	80 f5		bra $faf5			BRA	PUTSY
.fb00					PUTSY1
.fb00	60		rts				RTS
.fb01					NAT_SAV_CONTEXT
.fb01	e2 20		sep #$20			SEP	#M_FLAG			;
.fb03	8d 02 7e	sta $7e02			STA	M_A
.fb06	eb		xba				XBA
.fb07	8d 03 7e	sta $7e03			STA	M_B
.fb0a	68		pla				PLA
.fb0b	8d 01 7e	sta $7e01			STA	M_FLAGS			; Pull flags put on stack by BRK instruction
.fb0e	29 30		and #$30			AND #$30			; Figure which E mode
.fb10	4a		lsr a				LSR	A
.fb11	4a		lsr a				LSR	A
.fb12	4a		lsr a				LSR	A
.fb13	4a		lsr a				LSR	A
.fb14	8d 00 7e	sta $7e00			STA	M_EMODE			; Remember the flag combinations
.fb17					BNICP1
.fb17	c2 10		rep #$10			REP	#X_FLAG			; 16 bit index, binary mode
.fb19	8e 04 7e	stx $7e04			STX	M_X
.fb1c	8c 06 7e	sty $7e06			STY	M_Y
.fb1f	fa		plx				PLX					; Pull PC15..0 return address off stack
.fb20	ca		dex				DEX					; points past BRK... restore to point to BRK for continue
.fb21	ca		dex				DEX					; " Now we're pointing at BRK.  Handler should restore the byte here
.fb22	8e 0c 7e	stx $7e0c			STX	M_PC			; save PC=*(BRK instruction)
.fb25	68		pla				PLA					; Get PBR of code
.fb26	8d 0e 7e	sta $7e0e			STA	M_PBR
.fb29	ba		tsx				TSX					; Now we're pulled everything off stack - it's pre-BRK position
.fb2a	8e 08 7e	stx $7e08			STX	M_SP
.fb2d	0b		phd				PHD					; save DPR (zero page pointer)
.fb2e	fa		plx				PLX
.fb2f	8e 0a 7e	stx $7e0a			STX	M_DPR
.fb32	8b		phb				PHB					; Save
.fb33	68		pla				PLA
.fb34	8d 0f 7e	sta $7e0f			STA	M_DBR
.fb37					BNRET
.fb37	a9 00		lda #$00			LDA	#0				; Monitor is in bank #0
.fb39	48		pha				PHA					; push PBR=0
.fb3a	a2 00 f8	ldx #$f800			LDX	#START
.fb3d	da		phx				PHX					; push "return address"
.fb3e	08		php				PHP					; save flags
.fb3f	40		rti				RTI					; Jump to monitor entry
.fb40					NMI_ISR
.fb40	40		rti				RTI
.fb41					IRQ_ISR
.fb41	40		rti				RTI
.fb42					BRK_IRQ_EMU
.fb42	8d 02 7e	sta $7e02			STA	M_A			; Save A and B
.fb45	eb		xba				XBA
.fb46	8d 03 7e	sta $7e03			STA	M_B
.fb49	a9 ff		lda #$ff			LDA	#$FF			; We came from emulation mode.  Remember that
.fb4b	8d 00 7e	sta $7e00			STA	M_EMODE			; E=1; we came from Emulation mode
.fb4e	68		pla				PLA				; get flags from stack
.fb4f	48		pha				PHA				; and put them back!
.fb50	29 10		and #$10			AND	#BRK_FLAG		; Check for BRK flag
.fb52	d0 09		bne $fb5d			BNE	BRK_SAV_CONTEXT		; continue break handling
.fb54	ad 03 7e	lda $7e03			LDA	M_B			; restore A & B
.fb57	eb		xba				XBA
.fb58	ad 02 7e	lda $7e02			LDA	M_A
.fb5b	80 44		bra $fba1			BRA	IRQ_EMU_ISR		; EMU mode IRQ handler
.fb5d					BRK_SAV_CONTEXT
.fb5d	68		pla				PLA				; Get flags off stack
.fb5e	8d 01 7e	sta $7e01			STA	M_FLAGS			; Pull flags put on stack by BRK instruction
.fb61	8e 04 7e	stx $7e04			STX	M_X
.fb64	9c 05 7e	stz $7e05			STZ	M_X+1
.fb67	8c 06 7e	sty $7e06			STY	M_Y
.fb6a	9c 07 7e	stz $7e07			STZ	M_Y+1
.fb6d	68		pla				PLA				; Pull PC7..0
.fb6e	38		sec				SEC
.fb6f	e9 02		sbc #$02			SBC	#$02			; Subtract 2 from low address
.fb71	8d 0c 7e	sta $7e0c			STA	M_PC			; store as low PC
.fb74	85 44		sta $44				STA EA_L
.fb76	68		pla				PLA				; Get high PC PLA sets N and Z but leaves C alone fortunately
.fb77	e9 00		sbc #$00			SBC	#$00			; take care of any borrow from low PC
.fb79	8d 0d 7e	sta $7e0d			STA	M_PC+1			;
.fb7c	85 45		sta $45				STA EA_H
.fb7e	9c 0e 7e	stz $7e0e			STZ	M_PBR			; Probably garbage, but slightly possibly holding future context
.fb81	ba		tsx				TSX					; Now we're pulled everything off stack - it's pre-BRK position
.fb82	8e 08 7e	stx $7e08			STX	M_SP
.fb85	a9 01		lda #$01			LDA	#$01
.fb87	8d 09 7e	sta $7e09			STA	M_SP+1
.fb8a	0b		phd				PHD				; save DPR (zero page pointer).  Again, precautionary context save,
.fb8b	68		pla				PLA				; probably not important to user process (unless maybe it switches to native
.fb8c	8d 0b 7e	sta $7e0b			STA	M_DPR+1			; at some point when it resumes.
.fb8f	68		pla				PLA
.fb90	8d 0a 7e	sta $7e0a			STA	M_DPR
.fb93	9c 0f 7e	stz $7e0f			STZ	M_DBR
.fb96	9c 0e 7e	stz $7e0e			STZ	M_PBR
.fb99	a9 f8		lda #$f8			LDA	#>START
.fb9b	48		pha				PHA
.fb9c	a9 00		lda #$00			LDA	#<START
.fb9e	48		pha				PHA
.fb9f	08		php				PHP
.fba0	40		rti				RTI
.fba1					IRQ_EMU_ISR
.fba1	40		rti				RTI
.fba2					RES_CONTEXT
.fba2	ad 00 7e	lda $7e00			LDA	M_EMODE				; emulation mode?
.fba5	d0 2e		bne $fbd5			BNE	RES_EMU				; If 0, we're restoring to a native context
.fba7					RES_NATIVE
.fba7	c2 18		rep #$18			REP	#(X_FLAG | D_FLAG)
.fba9	e2 20		sep #$20			SEP	#(M_FLAG)
.fbab	18		clc				CLC
.fbac	fb		xce				XCE										; go to native mode
.fbad	ae 08 7e	ldx $7e08			LDX	M_SP								; 16 bit SP
.fbb0	9a		txs				TXS										; Restore the stack - target stack but we're pointing to free space so OK
.fbb1	ad 0e 7e	lda $7e0e			LDA	M_PBR
.fbb4	48		pha				PHA
.fbb5	ae 0c 7e	ldx $7e0c			LDX	M_PC
.fbb8	da		phx				PHX										; push
.fbb9	ad 01 7e	lda $7e01			LDA	M_FLAGS
.fbbc	48		pha				PHA
.fbbd	ae 0a 7e	ldx $7e0a			LDX	M_DPR
.fbc0	da		phx				PHX
.fbc1	ad 0f 7e	lda $7e0f			LDA	M_DBR
.fbc4	48		pha				PHA
.fbc5	ae 04 7e	ldx $7e04			LDX	M_X
.fbc8	ac 06 7e	ldy $7e06			LDY	M_Y
.fbcb	ad 03 7e	lda $7e03			LDA	M_B
.fbce	eb		xba				XBA
.fbcf	ad 02 7e	lda $7e02			LDA	M_A
.fbd2	ab		plb				PLB
.fbd3	2b		pld				PLD
.fbd4	40		rti				RTI										; RTI(24) Must use RTI to restore flags & Full 24 bit PC.  No other way!(?)
.fbd5					RES_EMU
.fbd5	e2 30		sep #$30			SEP	#(X_FLAG | M_FLAG)
.fbd7	c2 08		rep #$08			REP	#(D_FLAG)
.fbd9	38		sec				SEC
.fbda	fb		xce				XCE					; we're in emulation mode, 8 bit everything
.fbdb	ae 08 7e	ldx $7e08			LDX	M_SP			; only LSByte matters
.fbde	9a		txs				TXS					; Now in target stack context (but Ok to add below/free, before returning)
.fbdf	9c 0e 7e	stz $7e0e			STZ	M_PBR			; ? Not used AFAIK in emulation mode; should not be restored if not zero?
.fbe2	ad 0f 7e	lda $7e0f			LDA	M_DBR
.fbe5	48		pha				PHA
.fbe6	ab		plb				PLB					; Set up the DBR
.fbe7	ae 04 7e	ldx $7e04			LDX	M_X
.fbea	ac 06 7e	ldy $7e06			LDY	M_Y
.fbed	ad 0b 7e	lda $7e0b			LDA	M_DPR+1
.fbf0	eb		xba				XBA
.fbf1	ad 0a 7e	lda $7e0a			LDA	M_DPR
.fbf4	5b		tcd				TCD					; Always 16 bits; DPR=B*256+A
.fbf5	ad 0d 7e	lda $7e0d			LDA	M_PC+1			; Set up the RTI frame, PCH
.fbf8	48		pha				PHA
.fbf9	ad 0c 7e	lda $7e0c			LDA	M_PC			; Set up the RTI frame, PCL
.fbfc	48		pha				PHA
.fbfd	ad 01 7e	lda $7e01			LDA	M_FLAGS
.fc00	48		pha				PHA
.fc01	ad 0b 7e	lda $7e0b			LDA	M_DPR+1
.fc04	48		pha				PHA
.fc05	ad 0a 7e	lda $7e0a			LDA	M_DPR
.fc08	48		pha				PHA
.fc09	ad 0f 7e	lda $7e0f			LDA	M_DBR
.fc0c	48		pha				PHA
.fc0d	ad 03 7e	lda $7e03			LDA   M_B
.fc10	eb		xba		        XBA
.fc11	ad 02 7e	lda $7e02			LDA	  M_A
.fc14	ab		plb				PLB
.fc15	2b		pld				PLD
.fc16	40		rti				RTI					; RTI(16)
.ffe4					NCOP
>ffe4	00 f8						.word	START		; COP exception in native mode
.ffe6					NBRK
>ffe6	01 fb						.word	NAT_SAV_CONTEXT	; BRK: Save context (coming from native mode)
.ffe8					NABORT
>ffe8	00 f8						.word	START
.ffea					NNMI
>ffea	40 fb						.word	NMI_ISR		; NMI interrupt in native mode
.ffee					NIRQ
>ffee	41 fb						.word	IRQ_ISR 	;
.fff4					ECOP
>fff4	00 f8						.word	START		; COP exception in 65c02 emulation mode
.fff8					EABORT
>fff8	00 f8						.word	START
.fffa					ENMI
>fffa	00 f8						.word	START		; NMI interrupt in 65c02 emulation mode
.fffc					ERESET
>fffc	00 f8						.word	START		; RESET exception in all modes
.fffe					EIRQ
>fffe	42 fb						.word	BRK_IRQ_EMU		; Note: when enabling IRQ, must test and pick between IRQ and BRK

;******  End of listing


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
        LDX #0
        PHX
        PLD                     ; DPR = 0 
        LDA #0
        PHA     
        PLB                     ; Make sure data bank is 0
		JSR	INIT_FIFO			; initialize FIFO
CMD_INIT 	
		JSR	INIT_CMD_PROC			; Prepare processor state machine
CMD_LOOP
		JSR	CMD_PROC			; Run processor state machine
		BRA	CMD_LOOP			; then do it some more
; END main monitor loop

		
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
		BRA	RD_CMD
PCBC1		
		CMP	#2
		BNE	PCBC2
		BRA	WR_CMD
PCBC2	
		CMP	#3
		BNE	PCBC3
		BRL	GO_CMD
PCBC3	
		CMP	#4					; set (volatile) Breakpoint
		BNE	PCBC4
		BRL	SET_BP_CMD			; Breakpoint command	
PCBC4	
		CMP	#5					; Get context
		BNE	PCBC5
		BRL	GET_CONTEXT
PCBC5
        CMP #6                  ; Restore context
        BNE PCBC6
        BRL INIT_CONTEXT         ; Set context according to PC message (initialize context)
PCBC6   
        CMP #7
        BNE PCBC7
        BRL RES_CONTEXT         ; restore context
PCBC7	
		CMP	#'E'				; echo command
		BNE	PCBERR
		BRL	ECHO_CMD
PCBERR	
		JSR	SEND_NAK			; Unknown cmd
		RTS


; RDMEM[01][start-address-Low][start-address-high][start-address-bank][LEN]		; 
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
		
; WRMEM[02][start-address-Low][start-address-high][start-address-bank][b0][b1]...[bn]
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
	

; GO[03][start-address-low][start-address-high][start-address-bank]	
GO_CMD	
		LDA	CMD_BUF+1		; Note: this could be more efficient.  Make it work first.
		STA	EA_L
		LDA	CMD_BUF+2
		STA	EA_H
		LDA	CMD_BUF+3
		STA	EA_B
		JSR	SEND_ACK
		JML [EA_PTR]
		
; SETBP[04][start-address-low][start-address-high][start-addres-bank]
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

; GET_CONTEXT[05]
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

; INIT_CONTEXT [06][E-flag][Flags][A8][B8][XL][XH][YL][YH][SPL][SPH][DPRL][DPRH][PCL][PCH][PBR][DBR]
; From PC message, set up context.  E-flag is basically a don't care but retained to align with GET_CONTEXT
INIT_CONTEXT
        LDX #0
ICGN1   LDA CMD_BUF+1,X
        STA M_STATE,X
        INX
        CPX #16
        BNE ICGN1
        JSR SEND_ACK
        RTS

; RES_CONTEXT [07]
; From (hopefully) previously-saved context from prior BRK, reload registers and context.
; Always enters in native mode and will only work from native mode, but may exit to emulation mode.
; Note: this "returns" (exits!) to user context and only returns to monitor via a future BRK in user program context
RES_CONTEXT
; Set context Native mode
        JSR SEND_ACK
        CLC
        XCE             ; Should be redundant - monitor is always in native mode
        REP #(X_FLAG | M_FLAG)
        .al
        .xl
        LDA M_DPR
        TCD             ; Restore zero page to user context
        LDA M_SP        ; user context stack pointer (we'll push some stuff on there on RTI)
        TCS             ; but SP will be correct AFTER RTI executes
        LDX M_X
        LDY M_Y
 ; Figure out if emulation mode or native mode
        SEP #M_FLAG
        .as
        LDA M_EMODE
        CMP #$FF        ; If we came from emulation mode, must enter mode & NOT push/restore PBR
        BEQ SCHEMI
        LDA M_FLAGS
        PHA
        LDA M_PBR
        PHA
        LDA M_PC+1
        PHA
        LDA M_PC
        PHA
        LDA M_DBR
        PHA
        REP #M_FLAG
        .al
        LDA M_A             ;Get last PLB:data item BEFORE restoring DBR!
        PLB                 ;Must switch DBR last (stack is always in bank 00 so OK)
        RTI

; Set context Emulation mode - this could probably be optimized -- maybe someday
SCHEMI
        SEP #(X_FLAG | M_FLAG)
        .as
        .xs
        SEC
        XCE                 ; Must enter emulation mode for RTI to work correctly
        LDA M_FLAGS
        PHA
        LDA M_PC+1
        PHA
        LDA M_PC
        PHA
        LDA M_DBR
        PHA
        LDA M_B
        XBA 
        LDA M_A
        PLB
        RTI

		
; ['E'][data]
; replies [data]
; Used for polling CPU to check data path, and also to see if it's in the monitor or running a user program (no response)
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
        REP #(X_FLAG | M_FLAG)    ; No data loss if any were 8 bits before
        .xl
        .al
        ; Handle all 16 bit quantities first
        STA M_A                 ; Note:  if bytes, A must directly precede B in memory
        STX M_X
        STY M_Y
        TDC
        STA M_DPR
        TSC
        CLC
        ADC #4                  ; Calculate where stack was pointing prior to BRK
        STA M_SP
        LDA 2,S
        SEC                     ; calculate PC where BRK instruction was inserted
        SBC #2                  ;       "
        STA M_PC               
        SEP #M_FLAG
        .as                     ; Now handle the 8 bit quantitites
        PLA                     ; Get flags
        STA M_FLAGS
        AND #$30                ; Extract M and X flags
        LSR A
        LSR A
        LSR A
        LSR A 
        STA M_EMODE             ; Remember the mode (convenience for interpresting results for register dump)
        PLA                     ; discard return address
        PLA
        PLA                     ; Get PBR
        STA M_PBR               
        PHB
        PLA
        STA M_DBR
BNRET		
		LDA	#0				; Monitor is in bank #0
		PHA					; push PBR=0
		LDX	#START
		PHX					; push "return address"
		PHP					; save arbitrary flags (don't matter apart from stack alignment)
		RTI					; Jump to monitor entry (which resets all the flags and registers anyhow)


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
		
;VER_MSG
;		.text	CR,LF
;		.text  	"************************",CR,LF
;		.text	"*     BinMon v0.1      *",CR,LF
;		.text	"*     Ross Archer      *",CR,LF
;		.text	"*   MIT License Use    *",CR,LF
;		.text	"*   27 November 2025   *",CR,LF
;		.text	"************************",CR,LF
;		.text	0
; END main monitor loop


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


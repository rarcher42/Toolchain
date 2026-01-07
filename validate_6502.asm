; Assembled with 64TASS
;   64tass -c ~.asm -L ~.lst --intel-hex -o ~.hex
; or
;   64tass -c ~.asm -L ~.lst --s-record -o ~.hex 
; Put the above equates into an included file per peripheral or board

            .cpu    "w65c02"

        .INCLUDE "cpu_symbols.inc"
*   =   $0
TBLZ    .fill   256

*       =       $0200
TBL2    .fill   256

*       =       $0300
TBL3    .fill   256

*       =       $0400
TESTNO  .byte   ?


JMPVEC  .word   ?       
*   =   $2000

START   
        SEI
        CLD
        LDX     #$FF
        TXS
        LDY     #0
        JSR     TEST_FLAGS
        JSR     TEST_INDEX
        JSR     TEST_LOADS
        ;
        ;
        ; All tests completed OK.  Signal success and exit
        LDY     #0
        LDA     #0
        STA     TESTNO
        JMP     EXIT
        

; Test #1 - Validate basic flag operations
; (SET, CLEAR, BRANCH-on) 
; This is probably the most fundamental functionality
; to verify from the standpoint of minimizing 
; dependencies on other operations.
;
; Note: does NOT test reverse branches
; for BCC, BCS, BEQ, BNE, BMI, BPL,
; BVC, and BVS.  These are tested elsewhere,
; as part of logic and arithmetic operations
; because of dependencies that may result in
; false positives.  
; TEST #1 - flags and branches
TEST_FLAGS
        ; Standard preamble
        LDX     #1
        STX     TESTNO  ; Set the test number
        LDY     #0      ; No fail code (yet)
        ; End standard preamble
TFC000  LDA     #$80    ; Test N flag on load
        BMI     TFC001
        LDY     #1      ; N flag not set on LDA (or BMI fail)
        JMP     EXIT
TFC001  LDA     #$7F
        BPL     TFC002
        LDY     #2      ; N flag not cleared on LDA or BPL fail)
        JMP     EXIT

TFC002  LDX     #$80
        BMI     TFC003
        LDY     #3      ; N flag not set on LDX
        JMP     EXIT
TFC003  LDX     #$7F
        BPL     TFC004
        LDY     #4      ; N flag not cleared on LDX
        JMP     EXIT
        
TFC004  LDY     #$80
        BMI     TFC005
        LDY     #5      ; N flag no cleared on LDY
        JMP     EXIT    
TFC005  LDY     #$7F
        BPL     TFC100
        LDY     #6      ; N flag no set on LDY
        JMP     EXIT
        BRK
        BRK
        BRK     ; Intentional hazard for mispredicted branch target
        BRK
        BRK     ; Y will be error code #$7F in case of bad target calc
        BRK
        BRK
; Check Zero flag
TFC100  LDA     #$00    ; Test Z flag on load
        BEQ     TFC101
        LDY     #10     ; Z flag not set on LDA (or BEQ fail)
        JMP     EXIT
TFC101  LDA     #$01
        BNE     TFC102
        LDY     #11     ; Z flag not cleared on LDA or BNE fail)
        JMP     EXIT

TFC102  LDX     #$00
        BEQ     TFC103
        LDY     #12     ; Z flag not set on LDX
        JMP     EXIT
TFC103  LDX     #$01
        BNE     TFC104
        LDY     #13     ; Z flag not cleared on LDX
        JMP     EXIT
        
TFC104  LDY     #$00
        BEQ     TFC105
        LDY     #14     ; Z flag no cleared on LDY
        JMP     EXIT    
TFC105  LDY     #$01
        BNE     TFC200
        LDY     #15     ; Z flag no set on LDY
        JMP     EXIT

TFC200  SEC             
        BCS     TFC201
        LDY     #20
        JMP     EXIT
TFC201  CLC
        BCC     TFC300
        LDY     #21
        JMP     EXIT

TFC300  LDA     #%01000000      ; Set V flag without dependencies on ADC/SBC logic
        PHA                     ; (Since we're using JSR we'll assume stack & PHA works
        PLP                     ; (Have no zero-dependency way to validate V flag?)
        BVS     TFC301  
        LDY     #30             ; V flag in wrong bit position or PHA/PLP bug
        JMP     EXIT
TFC301  CLV
        BVC     TFC400
        LDY     #31
        JMP     EXIT
TFC400
        ; Now check the positional accuracy of all flags vs. real 6502
        ; for all testable bits already did V/bit 6 of flags.
        LDA     #%10000000      ; Set N flag by position only (bit 7)
        PHA                     ; (Since we're using JSR we'll assume stack & PHA works
        PLP                     ; (Have no zero-dependency way to validate V flag?)
        BMI     TFC401  
        LDY     #40
        JMP     EXIT
        ; Can't verify B, D or I directly because missing SET or test
TFC401  LDA     #%00000010      ; Set Z flag by position only (bit 1)
        PHA                     ; (Since we're using JSR we'll assume stack & PHA works
        PLP                     ; (Have no zero-dependency way to validate V flag?)
        BEQ     TFC402  
        LDY     #40
        JMP     EXIT
TFC402  LDA     #%00000001      ; Set C flag by position only (bit 0)
        PHA                     ; (Since we're using JSR we'll assume stack & PHA works
        PLP                     ; (Have no zero-dependency way to validate V flag?)
        BCS     TFC403  
        LDY     #40
        JMP     EXIT
TFC403  SED
        PHP
        PLA
        AND     #%00001000      ; Hopefully AND works with Z flag
        BNE     TFC404
        LDY     #51             ; SED fails or D flag in wrong place or 
        JMP     EXIT            ; AND doesn't set Z flag correctly
TFC404  CLD
        PHP
        PLA
        AND     #%00001000      
        BEQ     TFC405          ; CLD fails or D flag in wrong place or
        LDY     #52             ; AND doesn't set Z flag correctly
        JMP     EXIT
TFC405  
        SEI
        PHP
        PLA
        AND     #%00000100      ; Hopefully AND works with Z flag
        BNE     TFC406
        LDY     #61             ; SEI fails or I flag in wrong place or 
        JMP     EXIT            ; AND doesn't set Z flag correctly
TFC406  CLI
        PHP
        PLA
        AND     #%00000100      
        BEQ     TFC500          ; CLI fails or I flag in wrong place or
        LDY     #62             ; AND doesn't set Z flag correctly
        JMP     EXIT
TFC500
        LDY     #0      ; No failure code
        RTS             ; on to next test
        
        
; Before we can test loads and stores with indexing, we need to verify
; that INX, DEX, INY, and DEY do the increment and decrements,
; and set the N and Z flags correctly in response.
; It's probably the second-most fundamental functionality 
; to validate to minimize dependencies
TEST_INDEX
        ; Standard preamble
        LDX     #2
        STX     TESTNO  ; Set the test number
        LDY     #0      ; No fail code (yet)
        ; End standard preamble
        LDX     #0
        ; Make sure INX works right
TIX100  LDX     #$FF
        BMI     TIX101
        LDY     #1      ; LDX doesn't set N flag correctly
        JMP     EXIT
TIX101  INX
        BEQ     TIX102
        LDY     #2      ; X didn't increment or INX doesn't set Z flag correctly
        JMP     EXIT
TIX102  BPL     TIX103
        LDY     #3
        JMP     EXIT    ; INX didn't set N flag correctly
TIX103  DEX
        BMI     TIX104
        LDY     #4      ; DEX didn't decrement or didn't set sign flag
        JMP     EXIT
TIX104  INX             ; X should be 0
        INX             ; X should be 1
        DEX             ; X should be 0
        BEQ     TIX105
        LDA     #5      ; DEX didn't set Z flag 
        JMP     EXIT
; Test Y
TIX105  LDY     #$FF
        BMI     TIX106
        LDY     #11     ; LDX doesn't set N flag correctly
        JMP     EXIT
TIX106  INY
        BEQ     TIX107
        LDY     #12     ; X didn't increment or INX doesn't set Z flag correctly
        JMP     EXIT
TIX107  BPL     TIX108
        LDY     #13
        JMP     EXIT    ; INX didn't set N flag correctly
TIX108  DEY
        BMI     TIX109
        LDY     #14     ; DEX didn't decrement or didn't set sign flag
        JMP     EXIT
TIX109  INY             ; X should be 0
        INY             ; X should be 1
        DEY             ; X should be 0
        BEQ     TIX500
        LDA     #15     ; DEX didn't set Z flag 
        JMP     EXIT    
TIX500
        LDY     #0      ; No failure code
        RTS     



TEST_LOADS
        ; Standard preamble
        LDX     #3
        STX     TESTNO  ; Set the test number
        LDY     #0      ; No fail code (yet)
        ; End standard preamble
	LDA	#$42
	STA	$42
	STA	$0242
TLA101 	LDA	$42
	CMP	#$42
	BEQ	TLA102
	LDY	#1	; LDA ZP or CMP# bug
	JMP	EXIT
TLA102	LDA	$0242
	CMP	#$42
	BEQ	TLA103
	LDA	#2	; LDA ABS or CMP# bug
	JMP	EXIT
TLA103	LDX	#$42
	LDA	$00,X
	CMP	#$42
	BEQ	TLA104	
	LDY	#3	; CMP bug, or LA ZP,X index bug
	JMP	EXIT
TLA104	LDX	#$42
	LDA	$0200,X
	CMP	#$42
	BEQ	TLA105	
	LDY	#4	; CMP bug or LDA ABS,X index bug
	JMP	EXIT
TLA105	LDY	#$42
	LDA	$0200,Y
	CMP 	#$42
	BEQ	TLA201
	LDY	#5	; CMP bug or LDA ABS,Y index bug
	JMP	EXIT
	; Fill TBLZ with data
	; Zero page location X holds value X
	; Memory[0200+X] = X as well
TLA201 	LDX     #0
	LDY	#$FF
TLA202  TXA
        STA     TBLZ,X	; Table Z[i] = i
	STA	TBL2,X	; Table 2[i] = i
	STA	TBL3,Y	; Table 3[i] = 255 - i
	DEY
        INX
        BNE     TLA202
	; Test LDY addressing modes
	LDY	$88
	CPY	#$88
	BEQ	TLA203
	LDY	#11	; LDY ZP bug or CPY # bug
	JMP	EXIT
TLA203	LDY	$0248
	CPY	#$48
	BEQ	TLA204
	LDY	#12	; LDY ABS bug
	JMP 	EXIT
TLA204	LDX	#$65
	LDY	$00,X
	CPY	#$65
	BEQ	TLA205
	LDY	#13	; ZP,X bug
	JMP	EXIT
TLA205	LDX	#$99
	LDY	$0200,X
	CPY	#$99
	BEQ	TLA302
	LDY	#14	; ZP,X bug
	JMP	EXIT
	
	; Test LDX addressing modes
TLA302	LDX	$88
	CPX	#$88
	BEQ	TLA303
	LDY	#21	; LDY ZP bug or CPY # bug
	JMP	EXIT
TLA303	LDX	$0248
	CPX	#$48
	BEQ	TLA304
	LDX	#22	; LDY ABS bug
	JMP 	EXIT
TLA304	LDY	#$65
	LDX	$00,Y
	CPX	#$65
	BEQ	TLA305
	LDY	#23	; ZP,X bug
	JMP	EXIT
TLA305	LDY	#$99
	LDX	$0200,Y
	CPY	#$99
	BEQ	TLA306
	LDY	#24	; ZP,X bug
	JMP	EXIT	
TLA306 	
	; Do the indirect addressing modes for LDA
	LDX	#$02
	LDA 	($00,X)
	CMP	#$FD	; From location $0302 from (00+02) = 02 03 = 0302
	BEQ	TLA307
	LDA	#31
	JMP	EXIT
TLA307	LDY	#$42
	LDA	($02),Y ; = $0302+$42 = $0344 holds $FF-$42 = BB
	CMP	#$BB
	BEQ	TLA500
	LDA	#32
	JMP	EXIT
TLA500
        LDY     #0      ; No failure code
        RTS             
        

        
; We go to EXIT on first failure.  
; On entry:
; Y holds the reason code for failure or 0 if no failure
; TESTNO holds the TEST # that failed or 0 if all passed
;
; On exit:
; A holds the reason code for failure
; X holds the test case that fails
; Y holds $42 as a signature
;
        
EXIT    TYA                     ; Get fail code in A
        LDY     #$42
        LDX     TESTNO          ; X = Test case that failed (or 0 if passed)
        BRK



; Assembled with 64TASS
; 	64tass -c ~.asm -L ~.lst --intel-hex -o ~.hex
; or
;   64tass -c ~.asm -L ~.lst --s-record -o ~.hex 
; Put the above equates into an included file per peripheral or board

        	.cpu    "65816"

		.INCLUDE "cpu_symbols.inc"

* 	= 	$2000
START 	
        ; Native mode instructions
        .as
        .xs
		ORA ($30,X)     ;$01
        COP #10
        ;---- ORA 1,S
        TSB $30
        ORA $42
        ASL $22
        ORA [$33]
        PHP             ;$08
        ORA #$42      ;$09
        ASL A
        PHD         
        TSB $0300
        ORA $4242
        ASL $5150
        ORA $123124
HERE    BPL HERE       ;$10
        ORA ($42),y
        ORA ($42)
        ORA (1,S),Y
        TRB $19
        ORA $42,X
        ASL $43,X 
        ORA [$12],Y
        CLC             ;$18
        ORA $2A04,Y
        INC A 
        TCS
        TRB $1234
        ORA $0300,X 
        ASL $4242,X          
        ORA $402011,X 
        JSR HERE        ;$20
        AND ($69,X)
        JSL HERE
        AND 1,S
        BIT $20
        AND $30
        ROL $12
        AND [$10]
        PLP            ;$28
        AND #$FE      ;$29
        ROL A 
        PLD
        BIT $0BCD
        AND $0DEF
        ROL $5150
        AND $123456
        BMI HERE        ;$30
        AND ($20),Y
        AND ($20)
        AND (1,S),Y
        BIT $20,X 
        AND $21,X 
        ROL $69,X 
        AND [$30],Y
        SEC             ;$38
        AND $2A04,Y
        DEC A 
        TSC
        BIT $ABCD,X 
        AND $1234,X 
        ROL $5150,X 
        AND $123456,X 
        RTI             ;$40
        EOR ($22,X)
        WDM #$AA
        EOR 1,S
        MVP 1,2
        EOR $12
        LSR $33
        EOR [$27]
        PHA             ;$48
        EOR #$FF
        LSR A 
        PHK
        JMP $2A04
        EOR $ABCD
        LSR $2525
        EOR $123456
THERE
        BVC THERE       ; $50 
        
        EOR ($21),Y 
        EOR ($25)
        EOR (1,S),Y
        MVN 3,4
        EOR $19,X 
        LSR $18,X 
        EOR [$42],Y 
        CLI             ;$58
        EOR $0200,Y
        PHY
        TCD
        JML $123456
        EOR $1234,X 
        LSR $4231,X 
        EOR $123456,X 
        RTS             ;$60
        ADC ($50,X) 
        PER THERE
        ADC 1,S 
        STZ $30
        ADC $30
        ROR $59
        ADC [$89]
        PLA             ;$68
        ADC #$23
        ROR A 
        RTL     
        JMP ($FFFC)
        ADC $1234
        ROR $2345
        ADC $123456
        BVS THERE      ;$70
        ADC ($21),Y 
        ADC ($21)
        ADC (1,S),Y 
        STZ $12,X 
        ADC $11,X 
        ROR $55,X 
        ADC [$42],Y
        SEI           ;$78
        ADC $0100,Y
        PLY
        TDC
        JMP ($8080,X)
        ADC $1234,X     
        ROR $3456,X 
        ADC $123456,X 
T01
        BRA T01     ;$80
        STA ($00,X)
        BRL T01
        STA 1,S 
        STY $21
        STA $64
        STX $99
        STA [$55]
        DEY  
        BIT #$31
        TXA         
        PHB 
        STY $5150
        STA $1234
        STX $2345
        STA $123456
        BCC T01     ;$90
        STA ($22),Y 
        STA ($AA)
        STA (1,S),Y
        STY $55,X
        STA $55,X 
        STX $55,Y
        STA [$42],Y
        TYA          ;$98
        STA $1200,Y
        TXS
        TXY
        STZ $5150
        STA $0500,X 
        STZ $5555,X 
        STA $123456,X 
        LDY #$44    ;$A0
        LDA ($27,X)
        LDX #$24      ;$A2
        LDA 5,S 
        LDY $55
        LDA $68
        LDX $88
        LDA [$20]
        TAY         ;$A8
        LDA #$AB  ;$A9
        TAX
        PLB
        LDY $1234
        LDA $1234
        LDX $1234
        LDA $12345
T02
        BCS T02     ;$B0
        LDA ($88),Y
        LDA ($9A)
        LDA (3,S),Y 
        LDY $50,X 
        LDA $50,X 
        LDX $50,Y
        LDA [$40],Y
        CLV             ;$B8
        LDA $2525,Y
        TSX
        TYX
        LDY $0211,X 
        LDA $2101,X 
        LDX $2012,Y 
        LDA $123456,X 
        CPY #$42      ; $C0
        CMP ($50,X)
        REP #$30
        CMP 1,S
        CPY $22
        CMP $33
        DEC $44
        CMP [$42]
        INY             ; $C8
        CMP #$51      ; Are you nuts?
        DEX
        WAI
        CPY $4242
        CMP $4141
        DEC $2525
        CMP $125050
T03
        BNE T03         ;$D0
        CMP ($00),Y 
        CMP ($01)
        CMP (1,S),Y 
        PEI $31
        CMP $99,X 
        DEC $AA,X 
        CMP [$33],Y 
        CLD             ;$D8
        CMP $3124,Y 
        PHX             
        STP
        JML [$1234]
        CMP $5000,X 
        DEC $5150,X 
        CMP $123456,X 
        CPX #$FF      ; $E0
        SBC ($01,X)
        SEP #$00
        SBC 1,S 
        CPX $33
        SBC $99
        INC $28
        SBC [$42]
        INX             ;$E8
        SBC #$12
        NOP
        XBA
        CPX $3124
        SBC $5101
        INC $2222
        SBC $123456
        BEQ T03         ;$F0
        SBC ($94),Y 
        SBC ($88)
        SBC (4,S),Y 
        PEA T03
        SBC $44,X 
        INC $44,X 
        SBC [$42],Y 
        SED             ;$F8
        SBC $3141,Y 
        PLX
        XCE
        JSR ($8085,X)
        SBC $9980,X
        INC $9900,X
        SBC $123456,X   ;$FF
        NOP


        ; Emulation mode instructions (copied with deletions from .cpu "65c02"
        BRK



        ; 65c02 only instructions
        BRK
.end
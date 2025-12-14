; Assembled with 64TASS
; 	64tass -c ~.asm -L ~.lst --intel-hex -o ~.hex
; or
;   64tass -c ~.asm -L ~.lst --s-record -o ~.hex 
; Put the above equates into an included file per peripheral or board

        	.cpu    "w65c02"

		.INCLUDE "cpu_symbols.inc"

* 	= 	$2000
START 	
		ORA ($30,X)     ;$01
        ;COP #10         ;$02
        ;ORA 1,S         ;$03
        TSB $30         ;$04
        ORA $42         ;$05
        ASL $22         ;$06
        ;ORA [$33]       ;$07
        PHP             ;$08
        ORA #$42        ;$09
        ASL A           ;$0A
        ;PHD             ;$0B
        TSB $0300       ;$0C
        ORA $4242       ;$0D
        ASL $5150       ;$0E
        ;ORA $123124     ;$0F
HERE    BPL HERE        ;$10
        ORA ($42),y     ;$11
        ORA ($42)       ;$12
        ;ORA (1,S),Y     ;$13
        TRB $19         ;$14
        ORA $42,X       ;$15
        ASL $43,X       ;$16
        ;ORA [$12],Y     ;$17
        CLC             ;$18
        ORA $2A04,Y     ;$19
        INC A           ;$1A
        ;TCS             ;1B
        TRB $1234       ;1C
        ORA $0300,X     ;1D
        ASL $4242,X     ;1E      
        ;ORA $402011,X   ;1F
        JSR HERE        ;$20
        AND ($69,X)     ;21
        ;JSL HERE        ;22
        ;AND 1,S         ;23
        BIT $20         ;24
        AND $30         ;25
        ROL $12         ;26
        ;AND [$10]       ;27
        PLP             ;$28
        AND #$FE        ;$29
        ROL A           ;2A 
        ;PLD             ;2B
        BIT $0BCD       ;2C
        AND $0DEF       ;2D
        ROL $5150       ;2E
        ;AND $123456     ;2F
        BMI HERE        ;$30
        AND ($20),Y     ;31
        AND ($20)       ;32
        ;AND (1,S),Y     ;33
        BIT $20,X       ;34
        AND $21,X       ;35
        ROL $69,X       ;36
        ;AND [$30],Y     ;37
        SEC             ;$38
        AND $2A04,Y     ;39
        DEC A           ;3A
        ;TSC             ;3B
        BIT $ABCD,X     ;3C
        AND $1234,X     ;3D
        ROL $5150,X     ;3E
        ;AND $123456,X   ;3F
        RTI             ;$40
        ;EOR ($22,X)     ;41
        ;WDM #$AA        ;42
        ;EOR 1,S         ;43
        ;MVP 1,2         ;44
        EOR $12         ;45
        LSR $33         ;46
        ;EOR [$27]       ;47
        PHA             ;$48
        EOR #$FF        ;49
        LSR A           ;4A
        ;PHK             ;4B
        JMP $2A04       ;4C
        EOR $ABCD       ;4D
        LSR $2525       ;4E
        ;EOR $123456     ;4F
THERE
        BVC THERE       ; $50 
        
        EOR ($21),Y     ;51 
        EOR ($25)       ;52
        ;EOR (1,S),Y     ;53
        ;MVN 3,4         ;54
        EOR $19,X       ;55
        LSR $18,X       ;56
        ;EOR [$42],Y     ;57
        CLI             ;$58
        EOR $0200,Y     ;59
        PHY             ;5A
        ;TCD             ;5B
        ;JML $123456     ;5C
        EOR $1234,X     ;5D
        LSR $4231,X     ;5E
        ;EOR $123456,X   ;5F
        RTS             ;$60
        ADC ($50,X)     ;61
        ;PER THERE       ;62
        ;ADC 1,S         ;63
        STZ $30         ;64
        ADC $30         ;65
        ROR $59         ;66
        ;ADC [$89]       ;67
        PLA             ;$68
        ADC #$23        ;69
        ROR A           ;6A
        ;RTL             ;6B
        JMP ($FFFC)     ;6C
        ADC $1234       ;6D
        ROR $2345       ;6E
        ;ADC $123456     ;6F
        BVS THERE       ;$70
        ADC ($21),Y     ;71
        ADC ($21)       ;72
        ;ADC (1,S),Y     ;73
        STZ $12,X       ;74
        ADC $11,X       ;75
        ROR $55,X       ;76
        ;ADC [$42],Y     ;77
        SEI             ;78
        ADC $0100,Y     ;79
        PLY             ;7A
        ;TDC             ;7B
        JMP ($8080,X)   ;7C
        ADC $1234,X     ;7D
        ROR $3456,X     ;7E
        ;ADC $123456,X   ;7F
T01
        BRA T01         ;$80    
        STA ($00,X)     ;81
        ;BRL T01         ;82
        ;STA 1,S         ;83
        STY $21         ;84
        STA $64         ;85
        STX $99         ;86
        ;STA [$55]       ;87
        DEY             ;88
        BIT #$31        ;89
        TXA             ;8A
        ;PHB             ;8B
        STY $5150       ;8C
        STA $1234       ;8D
        STX $2345       ;8E
        ;STA $123456     ;8F
        BCC T01         ;$90
        STA ($22),Y     ;91
        STA ($AA)       ;92
        ;STA (1,S),Y     ;93
        STY $55,X       ;94
        STA $55,X       ;95
        STX $55,Y       ;96
        ;STA [$42],Y     ;97
        TYA             ;$98
        STA $1200,Y     ;99
        TXS             ;9A
        ;TXY             ;9B
        STZ $5150       ;9C
        STA $0500,X     ;9D
        STZ $5555,X     ;9E
        ;STA $123456,X   ;9F
        LDY #$44        ;$A0
        LDA ($27,X)     ;A1
        LDX #$24        ;A2
        ;LDA 5,S         ;A3
        LDY $55         ;A4
        LDA $68         ;A5
        LDX $88         ;A6
        ;LDA [$20]       ;A7
        TAY             ;$A8
        LDA #$AB        ;$A9
        TAX             ;AA
        ;PLB             ;AB
        LDY $1234       ;AC 
        LDA $1234       ;AD
        LDX $1234       ;AE
        ;LDA $12345      ;AF
T02
        BCS T02         ;$B0
        LDA ($88),Y     ;B1
        LDA ($9A)       ;B2
        ;LDA (3,S),Y     ;B3
        LDY $50,X       ;B4
        LDA $50,X       ;B5
        LDX $50,Y       ;B6
        ;LDA [$40],Y     ;B7
        CLV             ;$B8
        LDA $2525,Y     ;B9
        TSX             ;BA
        ;TYX             ;BB
        LDY $0211,X     ;BC
        LDA $2101,X     ;BD
        LDX $2012,Y     ;BE
        ;LDA $123456,X   ;BF
        CPY #$42        ;$C0
        CMP ($50,X)     ;C1
        ;REP #$30        ;C2
        ;CMP 1,S         ;C3
        CPY $22         ;C4
        CMP $33         ;C5
        DEC $44         ;C6
        ;CMP [$42]       ;C7
        INY             ; $C8
        CMP #$51        ;C9
        DEX             ;CA
        WAI             ;CB
        CPY $4242       ;CC
        CMP $4141       ;CD
        DEC $2525       ;CE
        ;CMP $125050     ;CF
T03
        BNE T03         ;$D0
        CMP ($00),Y     ;D1
        CMP ($01)       ;D2
        ;CMP (1,S),Y     ;D3
        ;PEI $31         ;D4    
        CMP $99,X       ;D5
        DEC $AA,X       ;D6
        ;CMP [$33],Y     ;D7
        CLD             ;$D8
        CMP $3124,Y     ;D9    
        PHX             ;DA
        STP             ;DB
        ;JML [$1234]     ;DC
        CMP $5000,X     ;DD
        DEC $5150,X     ;DE
        ;CMP $123456,X   ;DF
        CPX #$FF        ; $E0
        SBC ($01,X)     ;E1
        ;SEP #$00        ;E2
        ;SBC 1,S         ;E3
        CPX $33         ;E4
        SBC $99         ;E5
        INC $28         ;E6
        ;SBC [$42]       ;E7
        INX             ;$E8
        SBC #$12        ;E9
        NOP             ;EA
        ;XBA             ;EB
        CPX $3124       ;EC
        SBC $5101       ;ED
        INC $2222       ;EE
        ;SBC $123456     ;EF
        BEQ T03         ;$F0
        SBC ($94),Y     ;F1
        SBC ($88)       ;F2
        ;SBC (4,S),Y     ;F3
        ;PEA T03         ;F4
        SBC $44,X       ;F5
        INC $44,X       ;F6
        ;SBC [$42],Y     ;F7
        SED             ;$F8
        SBC $3141,Y     ;F9
        PLX             ;FA
        ;XCE             ;FB
        ;JSR ($8085,X)   ;FC
        SBC $9980,X     ;FD
        INC $9900,X     ;FE
        ;SBC $123456,X   ;$FF
        NOP


        ; Emulation mode instructions (copied with deletions from .cpu "65c02"
        BRK



        ; 65c02 only instructions
        BRK
.end
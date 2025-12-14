import random
import time
import sys
import serial
''' 
01 READBYTES:       01 SAL SAH SAB BCL BCH NLO N    (read N+1 bytes 1-256 for N=0..255)
    returns n+1 bytes
    
02 WRITEBYTES       02 SAL SAH SAB B0 B1 B2..... BN  (use CMD_IX to find last byte to write) - Max 256 bytes
    returns ACK
    
03 JUMPTO           03 SAL SAH SAB  
    returns ACK; anything the target program outputs to FIFO
    
04 SETBKPT          04 SAL SAH SAB
    returns replaced byte.  Store this away and restore it with a WRITEBYTES

05 GETCONTEXT       05
    returns         [E-flag][Flags][B][A][XL][XH][YL][YH][SPL][SPH][DPRL][DPRH][PCL][PCH][PBR][DBR]
'''

SER_PORT="COM4"

IMM=0
opcode_table = (
# 00
# {"MNEMONIC", M0X0-bytes, M0X1 bytes, M1X0 byres, M1X1 bytes, E-bytes, 65c02 bytes, 6502 bytes}
("BRK", IMM,  1,1,1,1,1,1), # BRK               ;$00
("ORA", IMM,  2,2,2,2,2,2), # ORA ($30,X)       ;$01
("COP", IMM,  2,2,2,2,0,0), # COP #10           ;$02
("ORA", IMM,  2,2,2,2,0,0), # ORA 1,S           ;$03 
("TSB", IMM,  2,2,2,2,2,0), # TSB $30           ;$04  
("ORA", IMM,  2,2,2,2,2,2), # ORA $42           ;$05
("ASL", IMM,  2,2,2,2,2,2), # ASL $22           ;$06
("ORA", IMM,  2,2,2,2,0,0), # ORA [$33]         ;$07

# 08
("PHP", IMM,  1,1,1,1,1,1),  # PHP             ;$08
("ORA", IMM,  3,3,2,3,3,3),  # ORA #$42        ;$09
("ASL", IMM,  1,1,1,1,1,1),  # ASL A           ;$0A
("PHD", IMM,  1,1,1,1,0,0),  # PHD             ;$0B
("TSB", IMM,  3,3,3,3,3,0),  # TSB $0300       ;$0C
("ORA", IMM,  3,3,3,3,3,3),  # ORA $4242       ;$0D
("ASL", IMM,  3,3,3,3,3,3),  # ASL $5150       ;$0E
("ORA", IMM,  4,4,4,4,0,0),  # ORA $123124     ;$0F
# 10
("BPL", IMM,  2,2,2,2,2,2),  # BPL HERE        ;$10
("ORA", IMM,  2,2,2,2,2,2),  # ORA ($42),y     ;$11
("ORA", IMM,  2,2,2,2,2,0),  # ORA ($42)       ;$12
("ORA", IMM,  2,2,2,2,0,0),  # ORA (1,S),Y     ;$13
("TRB", IMM,  2,2,2,2,2,0),  # TRB $19         ;$14
("ORA", IMM,  2,2,2,2,2,2),  # ORA $42,X       ;$15
("ASL", IMM,  2,2,2,2,2,2),  # ASL $43,X       ;$16
("ORA", IMM,  2,2,2,2,0,0),  # ORA [$12],Y     ;$17
# 18
("CLC", IMM,  1,1,1,1,1,1),  # CLC             ;$18
("ORA", IMM,  3,3,3,3,3,3),  # ORA $2A04,Y     ;$19
("INC", IMM,  1,1,1,1,1,0),  # INC A           ;$1A
("TCS", IMM,  1,1,1,1,0,0),  # TCS             ;1B
("TRB", IMM,  3,3,3,3,3,0),  # TRB $1234       ;1C
("ORA", IMM,  3,3,3,3,3,3),  # ORA $0300,X     ;1D
("ASL", IMM,  3,3,3,3,3,3),  # ASL $4242,X     ;1E
("ORA", IMM,  4,4,4,4,0,0),  # ORA $402011,X   ;1F
# 20
("JSR", IMM,  3,3,3,3,3,3),  # JSR HERE        ;$20 
("AND", IMM,  2,2,2,2,2,2),  # AND ($69,X)     ;21
("JSL", IMM,  4,4,4,4,0,0),  # JSL HERE        ;22
("AND", IMM,  2,2,2,2,0,0),  # AND 1,S         ;23
("BIT", IMM,  2,2,2,2,2,2),  # BIT $20         ;24
("AND", IMM,  2,2,2,2,2,2),  # AND $30         ;25
("ROL", IMM,  2,2,2,2,2,2),  # ROL $12         ;26
("AND", IMM,  2,2,2,2,0,0),  # AND [$10]       ;27
# 28
("PLP", IMM,  1,1,1,1,1,1),  # PLP             ;$28
("AND", IMM,  3,3,2,3,3,3),  # AND #$FE        ;$29
("ROL", IMM,  1,1,1,1,1,1),  # ROL A           ;2A
("PLD", IMM,  1,1,1,1,0,0),  # PLD             ;2B
("BIT", IMM,  3,3,3,3,3,3),  # BIT $0BCD       ;2C
("AND", IMM,  3,3,3,3,3,3),  # AND $0DEF       ;2D
("ROL", IMM,  3,3,3,3,3,3),  # ROL $5150       ;2E
("AND", IMM,  4,4,4,4,0,0),  # AND $123456     ;2F
#30               
("BMI", IMM,  2,2,2,2,2,2),  # BMI HERE        ;$30
("AND", IMM,  2,2,2,2,2,2),  # AND ($20),Y     ;31
("AND", IMM,  2,2,2,2,2,0),  # AND ($20)       ;32
("AND", IMM,  2,2,2,2,0,0),  # AND (1,S),Y     ;33
("BIT", IMM,  2,2,2,2,2,0),  # BIT $20,X       ;34
("AND", IMM,  2,2,2,2,2,2),  # AND $21,X       ;35
("ROL", IMM,  2,2,2,2,2,2),  # ROL $69,X       ;36
("AND", IMM,  2,2,2,2,0,0),  # AND [$30],Y     ;37
# 38
("SEC", IMM,  1,1,1,1,1,1),  # SEC             ;$38
("AND", IMM,  3,3,3,3,3,3),  # AND $2A04,Y     ;39
("DEC", IMM,  1,1,1,1,1,0),  # DEC A           ;3A
("TSC", IMM,  1,1,1,1,0,0),  # TSC             ;3B
("BIT", IMM,  3,3,3,3,3,0),  # BIT $ABCD,X     ;3C
("AND", IMM,  3,3,3,3,3,3),  # AND $1234,X     ;3D
("ROL", IMM,  3,3,3,3,3,3),  # ROL $5150,X     ;3E
("AND", IMM,  4,4,4,4,0,0),  # AND $123456,X   ;3F
# 40
("RTI", IMM,  1,1,1,1,1,1),  # RTI             ;$40
("EOR", IMM,  2,2,2,2,0,0),  # EOR ($22,X)     ;41
("WDM", IMM,  2,2,2,2,0,0),  # WDM #$AA        ;42
("EOR", IMM,  2,2,2,2,0,0),  # EOR 1,S         ;43
("MVN", IMM,  3,3,3,3,0,0),  # MVP 1,2         ;44
("EOR", IMM,  2,2,2,2,2,2),  # EOR $12         ;45
("LSR", IMM,  2,2,2,2,2,2),  # LSR $33         ;46 
("EOR", IMM,  2,2,2,2,0,0),  # EOR [$27]       ;47
# 48
("PHA", IMM,  1,1,1,1,1,1),  # PHA             ;$48
("EOR", IMM,  3,3,2,3,3,3),  # EOR #$FF        ;49
("LSR", IMM,  1,1,1,1,1,1),  # LSR A           ;4A
("PHK", IMM,  1,1,1,1,0,0),  # PHK             ;4B
("JMP", IMM,  3,3,3,3,3,3),  # JMP $2A04       ;4C
("EOR", IMM,  3,3,3,3,3,3),  # EOR $ABCD       ;4D
("LSR", IMM,  3,3,3,3,3,3),  # LSR $2525       ;4E
("EOR", IMM,  4,4,4,4,0,0),  # EOR $123456     ;4F
# 50
("BVC", IMM,  2,2,2,2,2,2),  # BVC THERE       ; $50
("EOR", IMM,  2,2,2,2,2,2),  # EOR ($21),Y     ;51
("EOR", IMM,  2,2,2,2,2,0),  # EOR ($25)       ;52
("EOR", IMM,  2,2,2,2,0,0),  # EOR (1,S),Y     ;53
("MVN", IMM,  3,3,3,3,0,0),  # MVN 3,4         ;54
("EOR", IMM,  2,2,2,2,2,2),  # EOR $19,X       ;55
("LSR", IMM,  2,2,2,2,2,2),  # LSR $18,X       ;56
("EOR", IMM,  2,2,2,2,0,0),  # EOR [$42],Y     ;57
# 58
("CLI", IMM,  1,1,1,1,1,1),  # CLI             ;$58
("EOR", IMM,  3,3,3,3,3,3),  # EOR $0200,Y     ;59
("PHY", IMM,  1,1,1,1,1,0),  # PHY             ;5A
("TCD", IMM,  1,1,1,1,0,0),  # TCD             ;5B
("JML", IMM,  4,4,4,4,0,0),  # JML $123456     ;5C
("EOR", IMM,  3,3,3,3,3,3),  # EOR $1234,X     ;5D
("LSR", IMM,  3,3,3,3,3,3),  # LSR $4231,X     ;5E
("EOR", IMM,  4,4,4,4,0,0),  # EOR $123456,X   ;5F
# 60
("RTS", IMM,  1,1,1,1,1,1),  # RTS             ;$60
("ADC", IMM,  2,2,2,2,2,2),  # ADC ($50,X)     ;61
("PER", IMM,  3,3,3,3,0,0),  # PER THERE       ;62
("ADC", IMM,  2,2,2,2,0,0),  # ADC 1,S         ;63
("STZ", IMM,  2,2,2,2,2,0),  # STZ $30         ;64
("ADC", IMM,  2,2,2,2,2,2),  # ADC $30         ;65
("ROR", IMM,  2,2,2,2,2,2),  # ROR $59         ;66
("ADC", IMM,  2,2,2,2,0,0),  # ADC [$89]       ;67
# 68
("PLA", IMM,  1,1,1,1,1,1),  # PLA             ;$68
("ADC", IMM,  3,3,2,3,3,3),  # ADC #$23        ;69
("ROR", IMM,  1,1,1,1,1,1),  # ROR A           ;6A
("RTL", IMM,  1,1,1,1,0,0),  # RTL             ;6B
("JMP", IMM,  3,3,3,3,3,3),  # JMP ($FFFC)     ;6C
("ADC", IMM,  3,3,3,3,3,3),  # ADC $1234       ;6D
("ROR", IMM,  3,3,3,3,3,3),  # ROR $2345       ;6E
("ADC", IMM,  4,4,4,4,0,0),  # ADC $123456     ;6F
# 70
("BVS", IMM,  2,2,2,2,2,2),  # BVS THERE       ;$70
("ADC", IMM,  2,2,2,2,2,2),  # ADC ($21),Y     ;71
("ADC", IMM,  2,2,2,2,2,2),  # ADC ($21)       ;72
("ADC", IMM,  2,2,2,2,0,0),  # ADC (1,S),Y     ;73
("STZ", IMM,  2,2,2,2,2,0),  # STZ $12,X       ;74
("ADC", IMM,  2,2,2,2,2,2),  # ADC $11,X       ;75
("ROR", IMM,  2,2,2,2,2,2),  # ROR $55,X       ;76
("ADC", IMM,  2,2,2,2,0,0),  # ADC [$42],Y     ;77
# 78
("SEI", IMM,  1,1,1,1,1,1),  # SEI             ;78
("ADC", IMM,  3,3,3,3,3,3),  # ADC $0100,Y     ;79
("PLY", IMM,  1,1,1,1,1,0),  # PLY             ;7A
("TDC", IMM,  1,1,1,1,0,0),  # TDC             ;7B
("JMP", IMM,  3,3,3,3,3,0),  # JMP ($8080,X)   ;7C
("ADC", IMM,  3,3,3,3,3,3),  # ADC $1234,X     ;7D
("ROR", IMM,  3,3,3,3,3,3),  # ROR $3456,X     ;7E
("ADC", IMM,  4,4,4,4,0,0),  # ADC $123456,X   ;7F
# 80
("BRA", IMM,  2,2,2,2,2,0),  # BRA T01         ;$80
("STA", IMM,  2,2,2,2,2,2),  # STA ($00,X)     ;81
("BRL", IMM,  3,3,3,3,0,0),  # BRL T01         ;82
("STA", IMM,  2,2,2,2,0,0),  # STA 1,S         ;83
("STY", IMM,  2,2,2,2,2,2),  # STY $21         ;84
("STA", IMM,  2,2,2,2,2,2),  # STA $64         ;85
("STX", IMM,  2,2,2,2,2,2),  # STX $99         ;86
("STA", IMM,  2,2,2,2,0,0),  # STA [$55]       ;87
# 88
("DEY", IMM,  1,1,1,1,1,1),  # DEY             ;88
("BIT", IMM,  3,3,2,3,3,0),  # BIT #$31        ;89
("TXA", IMM,  1,1,1,1,1,1),  # TXA             ;8A
("PHB", IMM,  1,1,1,1,0,0),  # PHB             ;8B
("STY", IMM,  3,3,3,3,3,3),  # STY $5150       ;8C
("STA", IMM,  3,3,3,3,3,3),  # STA $1234       ;8D
("STX", IMM,  3,3,3,3,3,3),  # STX $2345       ;8E
("STA", IMM,  4,4,4,4,0,0),  # STA $123456     ;8F
# 90
("BCC", IMM,  2,2,2,2,2,2),  # BCC T01         ;$90
("STA", IMM,  2,2,2,2,2,2),  # STA ($22),Y     ;91
("STA", IMM,  2,2,2,2,2,0),  # STA ($AA)       ;92
("STA", IMM,  2,2,2,2,0,0),  # STA (1,S),Y     ;93
("STY", IMM,  2,2,2,2,2,2),  # STY $55,X       ;94
("STA", IMM,  2,2,2,2,2,2),  # STA $55,X       ;95
("STX", IMM,  2,2,2,2,2,2),  # STX $55,Y       ;96
("STA", IMM,  2,2,2,2,0,0),  # STA [$42],Y     ;97
# 98
("TYA", IMM,  1,1,1,1,1,1),  # TYA             ;$98
("STA", IMM,  3,3,3,3,3,3),  # STA $1200,Y     ;99
("TXS", IMM,  1,1,1,1,1,1),  # TXS             ;9A
("TXY", IMM,  1,1,1,1,0,0),  # TXY             ;9B
("STZ", IMM,  3,3,3,3,3,0),  # STZ $5150       ;9C
("STA", IMM,  3,3,3,3,3,3),  # STA $0500,X     ;9D
("STZ", IMM,  3,3,3,3,3,0),  # STZ $5555,X     ;9E
("STA", IMM,  4,4,4,4,0,0),  # STA $123456,X   ;9F
# A0
("LDY", IMM,  3,2,3,2,2,2),  # LDY #$44        ;$A0
("LDA", IMM,  2,2,2,2,2,2),  # LDA ($27,X)     ;A1
("LDX", IMM,  3,2,3,2,3,3),  # LDX #$24        ;A2
("LDA", IMM,  2,2,2,2,0,0),  # LDA 5,S         ;A3
("LDY", IMM,  2,2,2,2,2,2),  # LDY $55         ;A4
("LDA", IMM,  2,2,2,2,2,2),  # LDA $68         ;A5
("LDX", IMM,  2,2,2,2,2,2),  # LDX $88         ;A6
("LDA", IMM,  2,2,2,2,0,0),  # LDA [$20]       ;A7

# A8
("TAY", IMM,  1,1,1,1,1,1),  # TAY             ;$A8
("LDA", IMM,  3,3,2,3,3,3),  # LDA #$AB        ;$A9
("TAX", IMM,  1,1,1,1,1,1),  # TAX             ;AA
("PLB", IMM,  1,1,1,1,0,0),  # PLB             ;AB
("LDY", IMM,  3,3,3,3,3,3),  # LDY $1234       ;AC
("LDA", IMM,  3,3,3,3,3,3),  # LDA $1234       ;AD
("LDX", IMM,  3,3,3,3,3,3),  # LDX $1234       ;AE
("LDA", IMM,  4,4,4,4,0,0),  # LDX $1234       ;AE

# B0
("BCS", IMM,  2,2,2,2,2,2),  # BCS T02         ;$B0  
("LDA", IMM,  2,2,2,2,2,2),  # LDA ($88),Y     ;B1
("LDA", IMM,  2,2,2,2,2,0),  # LDA ($9A)       ;B2
("LDA", IMM,  2,2,2,2,0,0),  # LDA (3,S),Y     ;B3
("LDY", IMM,  2,2,2,2,2,2),  # LDY $50,X       ;B4
("LDA", IMM,  2,2,2,2,2,2),  # LDA $50,X       ;B5
("LDX", IMM,  2,2,2,2,2,2),  # LDX $50,Y       ;B6
("LDA", IMM,  2,2,2,2,0,0),  # LDA [$40],Y     ;B7
# B8
("CLV", IMM,  1,1,1,1,1,1),  # CLV             ;$B8
("LDA", IMM,  3,3,3,3,3,3),  # LDA $2525,Y     ;B9
("TSX", IMM,  1,1,1,1,1,1),  # TSX             ;BA
("TYX", IMM,  1,1,1,1,0,0),  # TYX             ;BB
("LDY", IMM,  3,3,3,3,3,3),  # LDY $0211,X     ;BC
("LDA", IMM,  3,3,3,3,3,3),  # LDA $2101,X     ;BD
("LDX", IMM,  3,3,3,3,3,3),  # LDX $2012,Y     ;BE
("LDA", IMM,  4,4,4,4,0,0),  # LDA $123456,X   ;BF
# C0
("CPY", IMM,  3,2,3,2,3,3),  # CPY #$42        ;$C0
("CMP", IMM,  2,2,2,2,2,2),  # CMP ($50,X)     ;C1
("REP", IMM,  2,2,2,2,0,0),  # REP #$30        ;C2
("CMP", IMM,  2,2,2,2,0,0),  # CMP 1,S         ;C3
("CPY", IMM,  2,2,2,2,2,2),  # CPY $22         ;C4
("CMP", IMM,  2,2,2,2,2,2),  # CMP $33         ;C5
("DEC", IMM,  2,2,2,2,2,2),  # DEC $44         ;C6
("CMP", IMM,  2,2,2,2,0,0),  # CMP [$42]       ;C7
# C8
("INY", IMM,  1,1,1,1,1,1),  # INY             ; $C8
("CMP", IMM,  3,3,2,3,3,3),  # CMP #$51        ;C9
("DEX", IMM,  1,1,1,1,1,1),  # DEX             ;CA
("WAI", IMM,  1,1,1,1,1,0),  # WAI             ;CB
("CPY", IMM,  3,3,3,3,3,3),  # CPY $4242       ;CC
("CMP", IMM,  3,3,3,3,3,3),  # CMP $4141       ;CD
("DEC", IMM,  3,3,3,3,3,3),  # DEC $2525       ;CE
("CMP", IMM,  4,4,4,4,0,0),  # CMP $125050     ;CF
# D0
("BNE", IMM,  2,2,2,2,2,2),  # BNE T03         ;$D0
("CMP", IMM,  2,2,2,2,2,2),  # CMP ($00),Y     ;D1
("CMP", IMM,  2,2,2,2,2,0),  # CMP ($01)       ;D2
("CMP", IMM,  2,2,2,2,0,0),  # CMP (1,S),Y     ;D3
("PEI", IMM,  2,2,2,2,0,0),  # PEI $31         ;D4
("CMP", IMM,  2,2,2,2,2,2),  # CMP $99,X       ;D5
("DEC", IMM,  2,2,2,2,2,2),  # DEC $AA,X       ;D6
("CMP", IMM,  2,2,2,2,0,0),  # CMP [$33],Y     ;D7
# D8
("CLD", IMM,  1,1,1,1,1,1),  # CLD             ;$D8
("CMP", IMM,  3,3,3,3,3,3),  # CMP $3124,Y     ;D9
("PHX", IMM,  1,1,1,1,1,0),  # PHX             ;DA
("STP", IMM,  1,1,1,1,1,0),  # STP             ;DB
("JML", IMM,  3,3,3,3,0,0),  # JML [$1234]     ;DC
("CMP", IMM,  3,3,3,3,3,3),  # CMP $5000,X     ;DD
("DEC", IMM,  3,3,3,3,3,3),  # DEC $5150,X     ;DE
("CMP", IMM,  4,4,4,4,0,0),  # CMP $123456,X   ;DF
# E0
("CPX", IMM,  3,2,3,2,3,3),  # CPX #$FF        ;$E0
("SBC", IMM,  2,2,2,2,2,2),  # SBC ($01,X)     ;E1
("SEP", IMM,  2,2,2,2,0,0),  # SEP #$00        ;E2
("SBC", IMM,  2,2,2,2,0,0),  # SBC 1,S         ;E3
("CPX", IMM,  2,2,2,2,2,2),  # CPX $33         ;E4
("SBC", IMM,  2,2,2,2,2,2),  # SBC $99         ;E5
("INC", IMM,  2,2,2,2,2,2),  # INC $28         ;E6
("SBC", IMM,  2,2,2,2,0,0),  # SBC [$42]       ;E7
# E8
("INX", IMM,  1,1,1,1,1,1),  # INX             ;$E8
("SBC", IMM,  3,3,2,3,3,3),  # SBC #$12        ;E9
("NOP", IMM,  1,1,1,1,1,1),  # NOP             ;EA
("XBA", IMM,  1,1,1,1,0,0),  # XBA             ;EB
("INC", IMM,  3,3,3,3,3,3),  # CPX $3124       ;EC
("SBC", IMM,  3,3,3,3,3,3),  # SBC $5101       ;ED
("INC", IMM,  3,3,3,3,3,3),  # INC $2222       ;EE
("SBC", IMM,  4,4,4,4,0,0),  # SBC $123456     ;EF
# F0
("BEQ", IMM,  2,2,2,2,2,2),  # BEQ T03         ;$F0
("SBC", IMM,  2,2,2,2,2,2),  # SBC ($94),Y     ;F1
("SBC", IMM,  2,2,2,2,2,0),  # SBC ($88)       ;F2
("SBC", IMM,  2,2,2,2,0,0),  # SBC (4,S),Y     ;F3
("PEA", IMM,  3,3,3,3,0,0),  # PEA T03         ;F4
("SBC", IMM,  2,2,2,2,2,2),  # SBC $44,X       ;F5
("INC", IMM,  2,2,2,2,2,2),  # INC $44,X       ;F6
("SBC", IMM,  2,2,2,2,0,0),  # SBC [$42],Y     ;F7
# F8
("SED", IMM,  1,1,1,1,1,1),  # SED             ;$F8
("SBC", IMM,  3,3,3,3,3,3),  # SBC $3141,Y     ;F9
("PLX", IMM,  1,1,1,1,1,0),  # PLX             ;FA
("XCE", IMM,  1,1,1,1,0,0),  # XCE             ;FB
("JSR", IMM,  3,3,3,3,0,0),  # JSR ($8085,X)   ;FC
("SBC", IMM,  3,3,3,3,3,3),  # SBC $9980,X     ;FD
("INC", IMM,  3,3,3,3,3,3),  # INC $9900,X     ;FE
("SBC", IMM,  4,4,4,4,0,0)   # SBC $123456,X   ;$FF
)

class Frame:
    ''' Encapsulate a frame '''
    def __init__(self):
        self.SOF = 0x02
        self.ESC = 0x10
        self.EOF = 0x03
        self.SIZE_CMD_BUF = 512
    ''' 
        Encapsulate the payload to exclude SOF & EOF with ESC encoding 
        A wire frame begins with SOF and ends with EOF for ease of decoding 
    '''
    def wire_encode(self, rawpay):
        outbytes = bytes()
        outbytes += self.SOF.to_bytes(1)  # Start the wire frame
        for b in rawpay:
            if b == self.SOF: 
                outbytes += self.ESC.to_bytes(1)
                outbytes += 0x11.to_bytes(1)
            elif b == self.ESC:
                outbytes += self.ESC.to_bytes(1)
                outbytes += 0x12.to_bytes(1)
            elif b == self.EOF:
                outbytes += self.ESC.to_bytes(1)
                outbytes += 0x13.to_bytes(1) 
            else:
                outbytes += b.to_bytes(1)
        outbytes += self.EOF.to_bytes(1)  # End the wire frame
        return outbytes  

    ''' 
        Reverse payload encapsulation to yield original payload.  This
        involves discarding the intial SOF and terminal EOF, and 
        reversing all escaped data inside the payload to restore their values.
    '''
    def wire_decode(self, wire):
        state = 0
        inp = 0
        cmd_ptr = 0

        while True:
            if (cmd_ptr >= self.SIZE_CMD_BUF):
                print("OVERFLOW!!!!!!!!!!!!!!!!!!!!")
                state = 0
                exit(-1)
                continue

            # print("State = ", state)
            # INIT state
            if state == 0:
                state = 1
                cmd_ptr = 0
                outbytes = bytes()
                inp = 0
                error = False
                continue

            # AWAIT state
            elif state == 1:
                b = wire[inp]
                inp += 1
                if b == self.SOF:
                    state = 2
                continue
                
            # COLLECT state 
            elif state == 2:
                b = wire[inp]
                inp += 1
                if b == self.SOF:
                    cmd_ptr = 0
                    continue
                elif b == self.EOF:
                    state = 4
                    continue
                elif b == self.ESC:
                    state = 3
                    continue
                else:
                    outbytes += b.to_bytes(1)
                    cmd_ptr += 1
                    continue    
    
            # TRANSLATE state
            elif state == 3:
                b = wire[inp]
                inp += 1
                if b == self.SOF:
                    cmd_ptr = 0
                    outbytes = bytes()
                    continue
                elif b == self.EOF:
                    state = 0
                    continue
                elif b == 0x11:
                    outbytes += self.SOF.to_bytes(1)
                    cmd_ptr += 1
                    state = 2 
                    continue
                elif b == 0x12:
                    outbytes += self.ESC.to_bytes(1)
                    cmd_ptr += 1
                    state = 2
                    continue
                elif b == 0x13:
                    outbytes += self.EOF.to_bytes(1)
                    cmd_ptr += 1
                    state = 2
                    continue
                continue

            # PROCESS state
            elif state == 4:
                state = 0
                return outbytes
            # INVALID state
            else:
                print("INVALID STATE")
                sys.exit(-2)
                state = 0
                continue
               

class FIFO:
    def __init__(self, port='COM17', br=921600, parity=serial.PARITY_NONE, size=serial.EIGHTBITS,
                 stops=serial.STOPBITS_ONE, to=3.0):
        try:
            self.ser = serial.Serial(port=port, baudrate=br, parity=parity,
                            bytesize=size, stopbits=stops, timeout=to)
            self.open=True
        except:
            print("Error opening port")
            self.open = False

    def write(self, s):
        self.ser.write(s)
        return
        
    def read(self):
        outb = b''
        outb = self.ser.read_until(b'\x03')
        return outb
        
    def close(self):
        self.ser.close()
        return

# Create a bidirectional pipeline to the 65c816 CPU
class CPU_Pipe:
    def __init__ (self, port, rate):
        self.SERIAL_PORT = port
        self.fifo = FIFO(port, rate, serial.PARITY_NONE, serial.EIGHTBITS, serial.STOPBITS_ONE, 0.1)
        self.v = Frame()
        self.bp_list = list()
        self.fifo.read()     # Ditch any power on messages or noise bursts
        self.e_flag = False   # Assume something.  Correct it upon first status read
        self.m_flag = True
        self.x_flag = True

    def cmd_dialog(self, cmd):
        outf = self.v.wire_encode(cmd)
        self.fifo.write(outf)
        reinf = self.fifo.read()
        return self.v.wire_decode(reinf)
        
    def read_mem(self, sa_24, nbytes):
        nbytes -= 1             # 0-255 --> 1-256
        assert(nbytes < 256)
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        nbl = (nbytes & 0xFF).to_bytes(1, 'little')
        cmd = b'\x01'+sal+sah+sab+nbl
        return self.cmd_dialog(cmd)
        
    def write_mem(self, sa_24, data):
        assert(len(data) < 257)
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        cmd = b'\x02' + sal + sah + sab 
        for b in data:
            cmd += b.to_bytes(1, "little")
        return  self.cmd_dialog(cmd)

    def jump_long(self, sa_24):
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        cmd = b'\x03' + sal + sah + sab
        return self.cmd_dialog(cmd)
        
    def set_breakpoint(self, sa_24):
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        cmd = b'\x04' + sal + sah + sab
        value = self.cmd_dialog(cmd)       
        self.bp_list.append((sa_24, value))
        return value
       
    def replace_breaks(self):
        for bp in self.bp_list:
            print(bp)
            adr24 = bp[0]
            val8 = int.from_bytes(bp[1], 'little')
            print("Writing %06X: %02X" % (adr24, val8))
            self.write_mem(adr24, bp[1])
            self.bp_list.remove(bp)
        print("Exit bp_list=", self.bp_list)
    
    def get_registers(self):
        cmd = b'\x05'
        return self.cmd_dialog(cmd)
        
        
    def decode_flags(self, f, emode):
        s = "FLAGS(%02X)=" % f
        if f & 0x80:
            s+= "N"
        else:
            s+= "-"
            
        if f & 0x40:
            s += "V"
        else:
            s += "-"
            
        if f & 0x20:
            if emode == 0:
                s += "M"
            else:
                s += '1'
        else:
            s += "-"
        
        if f & 0x10:
            if emode == 0:
                s += "X"
            else:
                s += 'B'       # Break flag
        else:
            s += "-"
            
        if f & 0x8:
            s += "D"
        else:
            s += "-"
            
        if f & 0x4:
            s += "I"
        else:
            s += "-"
            
        if f & 0x2:
            s += "Z"
        else:
            s += "-"
        if f & 0x1:
            s += "C"
        else:
            s += "-"
        if (emode):
            s += " [E]"
        else:
            s += " [N]"
        return s
        
    def print_registers(self):
    # Refactor this, eventually
        # [E-flag][Flags][B][A][XL][XH][YL][YH][SPL][SPH][DPRL][DPRH][PCL][PCH][PBR][DBR]
        # 0:  E-Flag:  00= native FF=emulation
        # 1:  Flags      
        # 2:  A
        # 3:  B
        # 4:  XL, XH
        # 6:  YL, YH
        # 8:  SPL, SPH
        # 10: DPRL, DPRH
        # 12: PCL, PCH
        # 14: PBR
        # 15: DBR
        
        regs = self.get_registers()
        if regs[0] < 4:
            m_flag = (regs[0] & 0b10)
            x_flag = (regs[0] & 0b01)
            # Print native mode registers
            if m_flag:
                s = "A=%02X," % int.from_bytes(regs[2:3], "little")
                print(s, end="")
                s = "B=%02X," % int.from_bytes(regs[3:4], "little")
                print(s, end="")
            else:
                s = "C=%04X," % int.from_bytes(regs[2:4], "little")
                print(s, end="")
             # Print X and Y
            if x_flag:
                s = "X=%02X," % int.from_bytes(regs[4:5], "little")
                print(s, end="")
                s = "Y=%02X," % int.from_bytes(regs[6:7], "little")
                print(s, end="")
            else:
                s = "X=%04X," % int.from_bytes(regs[4:6], "little")
                print(s, end="")
                s = "Y=%04X," % int.from_bytes(regs[6:8], "little")
                print(s, end="")
            # Print SP
            s = "SP=%04X," % int.from_bytes(regs[8:10], "little")
            print(s, end="")
            s = "DPR=%04X," % int.from_bytes(regs[10:12], "little")
            print(s, end="")
            s = "PC=%04X," % int.from_bytes(regs[12:14], "little")
            print(s, end="")
            s = "PBR=%02X," % int.from_bytes(regs[14:15], "little")
            print(s, end="")
            s = "DBR=%02X," % int.from_bytes(regs[15:16], "little")
            print(s, end="")
            print(self.decode_flags(regs[1], False))
        else:
            s = "A=%02X," % int.from_bytes(regs[2:3], "little")
            print(s, end="")
            s = "B=%02X," % int.from_bytes(regs[3:4], "little")
            print(s, end="")
            s = "X=%02X," % int.from_bytes(regs[4:5], "little")
            print(s, end="")
            s = "Y=%02X," % int.from_bytes(regs[6:7], "little")
            print(s, end="")
            s = "SP=%04X," % (int.from_bytes(regs[8:9], "little") | 0x0100)
            print(s, end="")
            s = "PC=%04X," % int.from_bytes(regs[12:14], "little")
            print(s, end="")
            print(self.decode_flags(regs[1], True))
            
    def get_offset(self):
        # Get table offset according to settings of E=4+2, if E=0 2*M+X+2
        if (self.e_flag):
            return 3+2      # For now, use M1X1 entry
        if (self.m_flag):
            val = 2
        else:
            val = 0
        if (self.x_flag):
            val += 1
        return val+2    # Location of instruction length

    def get_instruction_at(self, addr):
        # Get the instruction byte
        op = self.read_mem(addr, 1)
        opv = int.from_bytes(op, "little")
        offset = self.get_offset()
        oplen = opcode_table[opv][offset]
        if (oplen > 1):
            instr = self.read_mem(addr, oplen)
        else:
            instr = op
        return instr


    def print_instruction_at(self, addr):
        instr = self.get_instruction_at(addr)
        opv = int.from_bytes(instr[0:1], "little")
        ns = self.mnemonic = opcode_table[opv][0]
        s = "%06X: " % addr
        s += ns
        s += dump_hex_str(instr)
        return s, instr


    def str_to_bytes(self, s):
        outb = b''
        for i in range(0, len(s), 2):
            x = int(s[i:i+2], 16)
            outb += x.to_bytes(1, 'little')
        return outb
        
    def send_srec(self, srec_fn):
        fh = open(srec_fn, "r")
        lines = fh.readlines()
        for li in lines:
            li = li.strip()
            rectype = li[0:2]
            if rectype == 'S0':
                # print("Rec(S0) - DISCARD")
                pass
            elif rectype == 'S1':
                ads = li[4:8]
                nstr = li[2:4]
                n = int(nstr, 16) - 3   # Subtract out checksum and 2 address bytes
                # print("Rec(S1) - 16 bits @$", ads) 
                addr = int(ads, 16)
                #print("ADDR = $%04X" % addr)
                datastr = li[8:-2]
                #print(len(datastr), datastr)
                barry = self.str_to_bytes(datastr)
                dump_hex(addr, barry)
                self.write_mem(addr, barry)
            elif rectype == 'S2':
                ads = li[4:10]
                nstr = li[2:4]
                n = int(nstr, 16) - 4   # Subtract checksum and 3 address bytes
                # print("Rec(S2) - 24 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%06X" % addr)
                datastr = li[10:-2]
                # print(len(datastr), datastr)
                barry = self.str_to_bytes(datastr)
                dump_hex(addr, barry)
                self.write_mem(addr, barry)
            elif rectype == 'S3':
                ads = li[4:12]
                nstr = li[2:4]
                n = int(nstr, 16) - 5   # Subtract checksum and 4 address bytes
                # print("Rec(S3) - 32 bits @$", ads)
                addr = int(ads, 16)
                # print("n = ", n)
                # print("ADDR = $%08X" % addr)
                datastr = li[12:-2]
                #print(len(datastr), datastr)
                barry = self.str_to_bytes(datastr)
                dump_hex(addr, barry)
                self.write_mem(addr, barry)
            elif rectype == 'S5':
                pass
            elif rectype == 'S7':
                ads = li[4:12]
                # print("Rec(S7) - 32 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%08X" % addr)
            elif rectype == 'S8':
                ads = li[4:10]
                # print("Rec(S8 - 24 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%06X" % addr)
            elif rectype == 'S9':
                ads = li[4:8]
                #print("Rec(S9) - 16 bits @$", ads)
                addr = int(ads, 16)
                #print("ADDR = $%04X" % addr)
            else:
                print("Unknown record type %s" % rectype)
        fh.close()
        return
        
        
def dump_hex(sa_24, data):
    # Note: if sa_24 = -1 then don't print leading address
    count = 0
    for b in data:
        if (count == 0) and sa_24 != -1:
            s = "\n%06X:" % sa_24
            print(s, end="")
        s = " %02X" % b
        print(s, end="")
        count += 1
        sa_24 += 1
        if count > 15:
            count = 0
    print("\n")
    return

def dump_hex_str(data):
    count = 0
    s = ""
    for b in data:
        s += " %02X" % b
        count += 1
        if count > 15:
            count = 0
            s += "\n"
    s += "\n"
    return s


        
def test_page_read_write(address):
    # Create a page of test data to write: Fill up a page with FF FE FD FC ... 01 00
    outb = b''
    for b in range(255, -1, -1):
        outb += b.to_bytes(1, "little")
    # OPEN a bidirectional CPU pipeline
    pipe = CPU_Pipe(SER_PORT, 921600)
   
    print("Writing data directly to memory")
    resp = pipe.write_mem(address, outb)
    print("Write returns: ", resp)
    
    print("Reading back the data")
    mem = pipe.read_mem(address, 256)
    dump_hex(address, mem)
    return 
 
def test_go(address):
    pipe = CPU_Pipe(SER_PORT, 921600)
    reply = pipe.jump_long(address)
    print(reply)
    return


if __name__ == "__main__":
    print(len(opcode_table))
    pipe = CPU_Pipe(SER_PORT, 921600)
    srec_fn = "allops_m1x1.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    done = False
    addr = 0x2000
    while not done:
        s, res = pipe.print_instruction_at(addr)
        print(s)
        nexti = int.from_bytes(res[0:1], 'little')
        if nexti == 0:
            done = True
        addr += len(res)
    
    exit(0)
    bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    pipe.replace_breaks()
    exit(0)
   
   
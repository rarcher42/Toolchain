import random
import time
import sys
import serial

''' f
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

# Addressing modes 
OP_NONE = 0
OP_A = 1 
OP_IMM = 2
OP_ABS = 3
OP_ABS_L = 4
OP_ABS_IND = 5 # "AI"
OP_ABS_IND_L = 6   # "AIL"
OP_ABS_X = 7 # "AX"
OP_ABS_Y = 8 # "AY"
OP_ABS_X_L = 9  # "AXL"
OP_ABS_X_IND = 10 # "AXI"
OP_ZP = 11
OP_ZP_IND = 12
OP_ZP_IND_L = 13
OP_ZP_X = 14
OP_ZP_Y = 15    # "ZY"
OP_ZP_XI = 16  
OP_ZP_IY = 17
OP_ZP_IY_L = 18 # ZILY
OP_REL = 19
OP_REL_L = 20
OP_SR = 21
OP_SR_IY = 22 # "SIY"
OP_2OPS = 23  # "TWO"
    

opcode_table = (
# 00
# {"MNEMONIC", (M0X0-bytes, M0X1 bytes, M1X0 byres, M1X1 bytes, 65c02 bytes, 6502 bytes}, (ADDRMODE)
("BRK",     (1,1,1,1,1,1), (OP_NONE)),          # BRK               ;$00
("ORA",     (2,2,2,2,2,2), (OP_ZP_XI)),         # ORA ($30,X)       ;$01
("COP",     (2,2,2,2,0,0), (OP_IMM)),           # COP #10           ;$02
("ORA",     (2,2,2,2,0,0), (OP_SR)),            # ORA 1,S           ;$03 
("TSB",     (2,2,2,2,2,0), (OP_ZP)),            # TSB $30           ;$04  
("ORA",     (2,2,2,2,2,2), (OP_ZP)),            # ORA $42           ;$05
("ASL",     (2,2,2,2,2,2), (OP_ZP)),            # ASL $22           ;$06
("ORA",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # ORA [$33]         ;$07
# 08
("PHP",     (1,1,1,1,1,1), (OP_NONE)),          # PHP              ;$08
("ORA",     (3,3,2,2,2,2), (OP_IMM)),           # ORA #$42         ;$09
("ASL",     (1,1,1,1,1,1), (OP_A)),             # ASL A            ;$0A
("PHD",     (1,1,1,1,0,0), (OP_NONE)),          # PHD              ;$0B
("TSB",     (3,3,3,3,3,0), (OP_ABS)),           # TSB $0300        ;$0C
("ORA",     (3,3,3,3,3,3), (OP_ABS)),           # ORA $4242        ;$0D
("ASL",     (3,3,3,3,3,3), (OP_ABS)),           # ASL $5150        ;$0E
("ORA",     (4,4,4,4,0,0), (OP_ABS_L)),         # ORA $123124      ;$0F
# 10
("BPL",     (2,2,2,2,2,2), (OP_REL)),           # BPL HERE         ;$10
("ORA",     (2,2,2,2,2,2), (OP_ZP_IY)),         # ORA ($42),y      ;$11
("ORA",     (2,2,2,2,2,0), (OP_ZP_IND)),        # ORA ($42)        ;$12
("ORA",     (2,2,2,2,0,0), (OP_SR_IY)),         # ORA (1,S),Y      ;$13
("TRB",     (2,2,2,2,2,0), (OP_ZP)),            # TRB $19          ;$14
("ORA",     (2,2,2,2,2,2), (OP_ZP_X)),          # ORA $42,X        ;$15
("ASL",     (2,2,2,2,2,2), (OP_ZP_X)),          # ASL $43,X        ;$16
("ORA",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # ORA [$12],Y      ;$17
# 18
("CLC",     (1,1,1,1,1,1), (OP_NONE)),          # CLC              ;$18
("ORA",     (3,3,3,3,3,3), (OP_ABS_Y)),         # ORA $2A04,Y      ;$19
("INC",     (1,1,1,1,1,0), (OP_A)),             # INC A            ;$1A
("TCS",     (1,1,1,1,0,0), (OP_NONE)),          # TCS              ;1B
("TRB",     (3,3,3,3,3,0), (OP_ABS)),           # TRB $1234        ;1C
("ORA",     (3,3,3,3,3,3), (OP_ABS_X)),         # ORA $0300,X      ;1D
("ASL",     (3,3,3,3,3,3), (OP_ABS_X)),         # ASL $4242,X      ;1E
("ORA",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # ORA $402011,X    ;1F
# 20
("JSR",     (3,3,3,3,3,3), (OP_ABS)),           # JSR HERE         ;$20 
("AND",     (2,2,2,2,2,2), (OP_ZP_XI)),         # AND ($69,X)      ;21
("JSL",     (4,4,4,4,0,0), (OP_ABS_L)),           # JSL HERE         ;22
("AND",     (2,2,2,2,0,0), (OP_SR)),            # AND 1,S          ;23
("BIT",     (2,2,2,2,2,2), (OP_ZP)),            # BIT $20          ;24
("AND",     (2,2,2,2,2,2), (OP_ZP)),            # AND $30          ;25
("ROL",     (2,2,2,2,2,2), (OP_ZP)),            # ROL $12          ;26
("AND",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # AND [$10]        ;27
# 28
("PLP",     (1,1,1,1,1,1), (OP_NONE)),          # PLP              ;$28
("AND",     (3,3,2,2,2,2), (OP_IMM)),           # AND #$FE         ;$29
("ROL",     (1,1,1,1,1,1), (OP_A)),             # ROL A            ;2A
("PLD",     (1,1,1,1,0,0), (OP_NONE)),          # PLD              ;2B
("BIT",     (3,3,3,3,3,3), (OP_ABS)),           # BIT $0BCD        ;2C
("AND",     (3,3,3,3,3,3), (OP_ABS)),           # AND $0DEF        ;2D
("ROL",     (3,3,3,3,3,3), (OP_ABS)),           # ROL $5150        ;2E
("AND",     (4,4,4,4,0,0), (OP_ABS_L)),         # AND $123456      ;2F
#30          
("BMI",     (2,2,2,2,2,2), (OP_REL)),           # BMI HERE         ;$30
("AND",     (2,2,2,2,2,2), (OP_ZP_IY)),         # AND ($20),Y      ;31
("AND",     (2,2,2,2,2,0), (OP_ZP_IND)),        # AND ($20)        ;32
("AND",     (2,2,2,2,0,0), (OP_SR_IY)),         # AND (1,S),Y      ;33
("BIT",     (2,2,2,2,2,0), (OP_ZP_X)),          # BIT $20,X        ;34
("AND",     (2,2,2,2,2,2), (OP_ZP_X)),          # AND $21,X        ;35
("ROL",     (2,2,2,2,2,2), (OP_ZP_X)),          # ROL $69,X        ;36
("AND",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # AND [$30],Y      ;37
# 38
("SEC",     (1,1,1,1,1,1), (OP_NONE)),          # SEC              ;$38
("AND",     (3,3,3,3,3,3), (OP_ABS_Y)),         # AND $2A04,Y      ;39
("DEC",     (1,1,1,1,1,0), (OP_A)),             # DEC A            ;3A
("TSC",     (1,1,1,1,0,0), (OP_NONE)),          # TSC              ;3B
("BIT",     (3,3,3,3,3,0), (OP_ABS_X)),         # BIT $ABCD,X      ;3C
("AND",     (3,3,3,3,3,3), (OP_ABS_X)),         # AND $1234,X      ;3D
("ROL",     (3,3,3,3,3,3), (OP_ABS_X)),         # ROL $5150,X      ;3E
("AND",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # AND $123456,X    ;3F
# 40
("RTI",     (1,1,1,1,1,1), (OP_NONE)),          # RTI              ;$40
("EOR",     (2,2,2,2,0,0), (OP_ZP_XI)),         # EOR ($22,X)      ;41
("WDM",     (2,2,2,2,0,0), (OP_IMM)),           # WDM #$AA         ;42
("EOR",     (2,2,2,2,0,0), (OP_SR)),            # EOR 1,S          ;43
("MVP",     (3,3,3,3,0,0), (OP_2OPS)),          # MVP 1,2          ;44
("EOR",     (2,2,2,2,2,2), (OP_ZP)),            # EOR $12          ;45
("LSR",     (2,2,2,2,2,2), (OP_ZP)),            # LSR $33          ;46 
("EOR",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # EOR [$27]        ;47
# 48
("PHA",     (1,1,1,1,1,1), (OP_NONE)),          # PHA              ;$48
("EOR",     (3,3,2,2,2,2), (OP_IMM)),           # EOR #$FF         ;49
("LSR",     (1,1,1,1,1,1), (OP_A)),             # LSR A            ;4A
("PHK",     (1,1,1,1,0,0), (OP_NONE)),          # PHK              ;4B
("JMP",     (3,3,3,3,3,3), (OP_ABS)),           # JMP $2A04        ;4C
("EOR",     (3,3,3,3,3,3), (OP_ABS)),           # EOR $ABCD        ;4D
("LSR",     (3,3,3,3,3,3), (OP_ABS)),           # LSR $2525        ;4E
("EOR",     (4,4,4,4,0,0), (OP_ABS_L)),         # EOR $123456      ;4F
# 50
("BVC",     (2,2,2,2,2,2), (OP_REL)),           # BVC THERE        ;$50
("EOR",     (2,2,2,2,2,2), (OP_ZP_IY)),         # EOR ($21),Y      ;51
("EOR",     (2,2,2,2,2,0), (OP_ZP_IND)),        # EOR ($25)        ;52
("EOR",     (2,2,2,2,0,0), (OP_SR_IY)),         # EOR (1,S),Y      ;53
("MVN",     (3,3,3,3,0,0), (OP_2OPS)),          # MVN 3,4          ;54
("EOR",     (2,2,2,2,2,2), (OP_ZP_X)),          # EOR $19,X        ;55
("LSR",     (2,2,2,2,2,2), (OP_ZP_X)),          # LSR $18,X        ;56
("EOR",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # EOR [$42],Y      ;57
# 58
("CLI",     (1,1,1,1,1,1), (OP_NONE)),          # CLI              ;$58
("EOR",     (3,3,3,3,3,3), (OP_ABS_Y)),         # EOR $0200,Y      ;59
("PHY",     (1,1,1,1,1,0), (OP_NONE)),          # PHY              ;5A
("TCD",     (1,1,1,1,0,0), (OP_NONE)),          # TCD              ;5B
("JML",     (4,4,4,4,0,0), (OP_ABS_L)),         # JML $123456      ;5C
("EOR",     (3,3,3,3,3,3), (OP_ABS_X)),         # EOR $1234,X      ;5D
("LSR",     (3,3,3,3,3,3), (OP_ABS_X)),         # LSR $4231,X      ;5E
("EOR",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # EOR $123456,X    ;5F
# 60
("RTS",     (1,1,1,1,1,1), (OP_NONE)),          # RTS              ;$60
("ADC",     (2,2,2,2,2,2), (OP_ZP_XI)),         # ADC ($50,X)      ;61
("PER",     (3,3,3,3,0,0), (OP_REL)),           # PER THERE        ;62
("ADC",     (2,2,2,2,0,0), (OP_SR)),            # ADC 1,S          ;63
("STZ",     (2,2,2,2,2,0), (OP_ZP)),            # STZ $30          ;64
("ADC",     (2,2,2,2,2,2), (OP_ZP)),            # ADC $30          ;65
("ROR",     (2,2,2,2,2,2), (OP_ZP)),            # ROR $59          ;66
("ADC",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # ADC [$89]        ;67
# 68
("PLA",     (1,1,1,1,1,1), (OP_NONE)),          # PLA              ;$68
("ADC",     (3,3,2,2,2,2), (OP_IMM)),           # ADC #$23         ;69
("ROR",     (1,1,1,1,1,1), (OP_A)),             # ROR A            ;6A
("RTL",     (1,1,1,1,0,0), (OP_NONE)),          # RTL              ;6B
("JMP",     (3,3,3,3,3,3), (OP_ABS_IND)),       # JMP ($FFFC)      ;6C
("ADC",     (3,3,3,3,3,3), (OP_ABS)),           # ADC $1234        ;6D
("ROR",     (3,3,3,3,3,3), (OP_ABS)),           # ROR $2345        ;6E
("ADC",     (4,4,4,4,0,0), (OP_ABS_L)),         # ADC $123456      ;6F
# 70
("BVS",     (2,2,2,2,2,2), (OP_REL)),           # BVS THERE        ;$70
("ADC",     (2,2,2,2,2,2), (OP_ZP_IY)),         # ADC ($21),Y      ;71
("ADC",     (2,2,2,2,2,2), (OP_ZP_IND)),        # ADC ($21)        ;72
("ADC",     (2,2,2,2,0,0), (OP_SR_IY)),         # ADC (1,S),Y      ;73
("STZ",     (2,2,2,2,2,0), (OP_ZP_X)),          # STZ $12,X        ;74
("ADC",     (2,2,2,2,2,2), (OP_ZP_X)),          # ADC $11,X        ;75
("ROR",     (2,2,2,2,2,2), (OP_ZP_X)),          # ROR $55,X        ;76
("ADC",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # ADC [$42],Y      ;77
# 78
("SEI",     (1,1,1,1,1,1), (OP_NONE)),          # SEI              ;78
("ADC",     (3,3,3,3,3,3), (OP_ABS_Y)),         # ADC $0100,Y      ;79
("PLY",     (1,1,1,1,1,0), (OP_NONE)),          # PLY              ;7A
("TDC",     (1,1,1,1,0,0), (OP_NONE)),          # TDC              ;7B
("JMP",     (3,3,3,3,3,0), (OP_ABS_X_IND)),     # JMP ($8080,X)    ;7C
("ADC",     (3,3,3,3,3,3), (OP_ABS_X)),         # ADC $1234,X      ;7D
("ROR",     (3,3,3,3,3,3), (OP_ABS_X)),         # ROR $3456,X      ;7E
("ADC",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # ADC $123456,X    ;7F
# 80
("BRA",     (2,2,2,2,2,0), (OP_REL)),           # BRA T01          ;$80
("STA",     (2,2,2,2,2,2), (OP_ZP_XI)),         # STA ($00,X)      ;81
("BRL",     (3,3,3,3,0,0), (OP_REL_L)),         # BRL T01          ;82
("STA",     (2,2,2,2,0,0), (OP_SR)),            # STA 1,S          ;83
("STY",     (2,2,2,2,2,2), (OP_ZP)),            # STY $21          ;84
("STA",     (2,2,2,2,2,2), (OP_ZP)),            # STA $64          ;85
("STX",     (2,2,2,2,2,2), (OP_ZP)),            # STX $99          ;86
("STA",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # STA [$55]        ;87
# 88
("DEY",     (1,1,1,1,1,1), (OP_NONE)),          # DEY              ;88
("BIT",     (3,3,2,2,2,0), (OP_IMM)),           # BIT #$31         ;89
("TXA",     (1,1,1,1,1,1), (OP_NONE)),          # TXA              ;8A
("PHB",     (1,1,1,1,0,0), (OP_NONE)),          # PHB              ;8B
("STY",     (3,3,3,3,3,3), (OP_ABS)),           # STY $5150        ;8C
("STA",     (3,3,3,3,3,3), (OP_ABS)),           # STA $1234        ;8D
("STX",     (3,3,3,3,3,3), (OP_ABS)),           # STX $2345        ;8E
("STA",     (4,4,4,4,0,0), (OP_ABS_L)),         # STA $123456      ;8F
# 90
("BCC",     (2,2,2,2,2,2), (OP_REL)),           # BCC T01          ;$90
("STA",     (2,2,2,2,2,2), (OP_ZP_IY)),         # STA ($22),Y      ;91
("STA",     (2,2,2,2,2,0), (OP_ZP_IND)),        # STA ($AA)        ;92
("STA",     (2,2,2,2,0,0), (OP_SR_IY)),         # STA (1,S),Y      ;93
("STY",     (2,2,2,2,2,2), (OP_ZP_X)),          # STY $55,X        ;94
("STA",     (2,2,2,2,2,2), (OP_ZP_X)),          # STA $55,X        ;95
("STX",     (2,2,2,2,2,2), (OP_ZP_Y)),          # STX $55,Y        ;96
("STA",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # STA [$42],Y      ;97
# 98
("TYA",     (1,1,1,1,1,1), (OP_NONE)),          # TYA              ;$98
("STA",     (3,3,3,3,3,3), (OP_ABS_Y)),         # STA $1200,Y      ;99
("TXS",     (1,1,1,1,1,1), (OP_NONE)),          # TXS              ;9A
("TXY",     (1,1,1,1,0,0), (OP_NONE)),          # TXY              ;9B
("STZ",     (3,3,3,3,3,0), (OP_ABS)),           # STZ $5150        ;9C
("STA",     (3,3,3,3,3,3), (OP_ABS_X)),         # STA $0500,X      ;9D
("STZ",     (3,3,3,3,3,0), (OP_ABS_X)),         # STZ $5555,X      ;9E
("STA",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # STA $123456,X    ;9F
# A0
("LDY",     (3,2,3,2,2,2), (OP_IMM)),           # LDY #$44         ;$A0
("LDA",     (2,2,2,2,2,2), (OP_ZP_XI)),         # LDA ($27,X)      ;A1
("LDX",     (3,2,3,2,2,2), (OP_IMM)),           # LDX #$24         ;A2
("LDA",     (2,2,2,2,0,0), (OP_SR)),            # LDA 5,S          ;A3
("LDY",     (2,2,2,2,2,2), (OP_ZP)),            # LDY $55          ;A4
("LDA",     (2,2,2,2,2,2), (OP_ZP)),            # LDA $68          ;A5
("LDX",     (2,2,2,2,2,2), (OP_ZP)),            # LDX $88          ;A6
("LDA",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # LDA [$20]        ;A7
# A8
("TAY",     (1,1,1,1,1,1), (OP_NONE)),          # TAY              ;$A8
("LDA",     (3,3,2,2,2,2), (OP_IMM)),           # LDA #$AB         ;$A9
("TAX",     (1,1,1,1,1,1), (OP_NONE)),          # TAX              ;AA
("PLB",     (1,1,1,1,0,0), (OP_NONE)),          # PLB              ;AB
("LDY",     (3,3,3,3,3,3), (OP_ABS)),           # LDY $1234        ;AC
("LDA",     (3,3,3,3,3,3), (OP_ABS)),           # LDA $1234        ;AD
("LDX",     (3,3,3,3,3,3), (OP_ABS)),           # LDX $1234        ;AE
("LDA",     (4,4,4,4,0,0), (OP_ABS_L)),         # LDX $123456        ;AF

# B0
("BCS",     (2,2,2,2,2,2), (OP_REL)),           # BCS T02          ;$B0  
("LDA",     (2,2,2,2,2,2), (OP_ZP_IY)),         # LDA ($88),Y      ;B1
("LDA",     (2,2,2,2,2,0), (OP_ZP_IND)),        # LDA ($9A)        ;B2
("LDA",     (2,2,2,2,0,0), (OP_SR_IY)),         # LDA (3,S),Y      ;B3
("LDY",     (2,2,2,2,2,2), (OP_ZP_X)),          # LDY $50,X        ;B4
("LDA",     (2,2,2,2,2,2), (OP_ZP_X)),          # LDA $50,X        ;B5
("LDX",     (2,2,2,2,2,2), (OP_ZP_Y)),          # LDX $50,Y        ;B6
("LDA",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # LDA [$40],Y      ;B7
# B8
("CLV",     (1,1,1,1,1,1), (OP_NONE)),          # CLV              ;$B8
("LDA",     (3,3,3,3,3,3), (OP_ABS_Y)),         # LDA $2525,Y      ;B9
("TSX",     (1,1,1,1,1,1), (OP_NONE)),          # TSX              ;BA
("TYX",     (1,1,1,1,0,0), (OP_NONE)),          # TYX              ;BB
("LDY",     (3,3,3,3,3,3), (OP_ABS_X)),         # LDY $0211,X      ;BC
("LDA",     (3,3,3,3,3,3), (OP_ABS_X)),         # LDA $2101,X      ;BD
("LDX",     (3,3,3,3,3,3), (OP_ABS_Y)),         # LDX $2012,Y      ;BE
("LDA",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # LDA $123456,X    ;BF
# C0
("CPY",     (3,2,3,2,2,2), (OP_IMM)),           # CPY #$42         ;$C0
("CMP",     (2,2,2,2,2,2), (OP_ZP_XI)),         # CMP ($50,X)      ;C1
("REP",     (2,2,2,2,0,0), (OP_IMM)),           # REP #$30         ;C2
("CMP",     (2,2,2,2,0,0), (OP_SR)),            # CMP 1,S          ;C3
("CPY",     (2,2,2,2,2,2), (OP_ZP)),            # CPY $22          ;C4
("CMP",     (2,2,2,2,2,2), (OP_ZP)),            # CMP $33          ;C5
("DEC",     (2,2,2,2,2,2), (OP_ZP)),            # DEC $44          ;C6
("CMP",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # CMP [$42]        ;C7
# C8
("INY",     (1,1,1,1,1,1), (OP_NONE)),          # INY              ; $C8
("CMP",     (3,3,2,2,2,2), (OP_IMM)),           # CMP #$51         ;C9
("DEX",     (1,1,1,1,1,1), (OP_NONE)),          # DEX              ;CA
("WAI",     (1,1,1,1,1,0), (OP_NONE)),          # WAI              ;CB
("CPY",     (3,3,3,3,3,3), (OP_ABS)),           # CPY $4242        ;CC
("CMP",     (3,3,3,3,3,3), (OP_ABS)),           # CMP $4141        ;CD
("DEC",     (3,3,3,3,3,3), (OP_ABS)),           # DEC $2525        ;CE
("CMP",     (4,4,4,4,0,0), (OP_ABS_L)),         # CMP $125050      ;CF
# D0
("BNE",     (2,2,2,2,2,2), (OP_REL)),           # BNE T03          ;$D0
("CMP",     (2,2,2,2,2,2), (OP_ZP_IY)),         # CMP ($00),Y      ;D1
("CMP",     (2,2,2,2,2,0), (OP_ZP_IND)),        # CMP ($01)        ;D2
("CMP",     (2,2,2,2,0,0), (OP_SR_IY)),         # CMP (1,S),Y      ;D3
("PEI",     (2,2,2,2,0,0), (OP_ZP)),            # PEI $31          ;D4
("CMP",     (2,2,2,2,2,2), (OP_ZP_X)),          # CMP $99,X        ;D5
("DEC",     (2,2,2,2,2,2), (OP_ZP_X)),          # DEC $AA,X        ;D6
("CMP",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # CMP [$33],Y      ;D7
# D8
("CLD",     (1,1,1,1,1,1), (OP_NONE)),          # CLD              ;$D8
("CMP",     (3,3,3,3,3,3), (OP_ABS_Y)),         # CMP $3124,Y      ;D9
("PHX",     (1,1,1,1,1,0), (OP_NONE)),          # PHX              ;DA
("STP",     (1,1,1,1,1,0), (OP_NONE)),          # STP              ;DB
("JML",     (3,3,3,3,0,0), (OP_ABS_IND_L)),     # JML [$1234]      ;DC
("CMP",     (3,3,3,3,3,3), (OP_ABS_X)),         # CMP $5000,X      ;DD
("DEC",     (3,3,3,3,3,3), (OP_ABS_X)),         # DEC $5150,X      ;DE
("CMP",     (4,4,4,4,0,0), (OP_ABS_X_L)),       # CMP $123456,X    ;DF
# E0
("CPX",     (3,2,3,2,2,2), (OP_IMM)),           # CPX #$FF         ;$E0
("SBC",     (2,2,2,2,2,2), (OP_ZP_XI)),         # SBC ($01,X)      ;E1
("SEP",     (2,2,2,2,0,0), (OP_IMM)),           # SEP #$00         ;E2
("SBC",     (2,2,2,2,0,0), (OP_SR)),            # SBC 1,S          ;E3
("CPX",     (2,2,2,2,2,2), (OP_ZP)),            # CPX $33          ;E4
("SBC",     (2,2,2,2,2,2), (OP_ZP)),            # SBC $99          ;E5
("INC",     (2,2,2,2,2,2), (OP_ZP)),            # INC $28          ;E6
("SBC",     (2,2,2,2,0,0), (OP_ZP_IND_L)),      # SBC [$42]        ;E7
# E8
("INX",     (1,1,1,1,1,1), (OP_NONE)),          # INX              ;$E8
("SBC",     (3,3,2,2,2,2), (OP_IMM)),           # SBC #$12         ;E9
("NOP",     (1,1,1,1,1,1), (OP_NONE)),          # NOP              ;EA
("XBA",     (1,1,1,1,0,0), (OP_NONE)),          # XBA              ;EB
("INC",     (3,3,3,3,3,3), (OP_ABS)),           # CPX $3124        ;EC
("SBC",     (3,3,3,3,3,3), (OP_ABS)),           # SBC $5101        ;ED
("INC",     (3,3,3,3,3,3), (OP_ABS)),           # INC $2222        ;EE
("SBC",     (4,4,4,4,0,0), (OP_ABS_L)),         # SBC $123456      ;EF
# F0
("BEQ",     (2,2,2,2,2,2), (OP_REL)),           # BEQ T03          ;$F0
("SBC",     (2,2,2,2,2,2), (OP_ZP_IY)),         # SBC ($94),Y      ;F1
("SBC",     (2,2,2,2,2,0), (OP_ZP_IND)),        # SBC ($88)        ;F2
("SBC",     (2,2,2,2,0,0), (OP_SR_IY)),         # SBC (4,S),Y      ;F3
("PEA",     (3,3,3,3,0,0), (OP_IMM)),           # PEA T03          ;F4
("SBC",     (2,2,2,2,2,2), (OP_ZP_X)),          # SBC $44,X        ;F5
("INC",     (2,2,2,2,2,2), (OP_ZP_X)),          # INC $44,X        ;F6
("SBC",     (2,2,2,2,0,0), (OP_ZP_IY_L)),       # SBC [$42],Y      ;F7
# F8
("SED",     (1,1,1,1,1,1), (OP_NONE)),          # SED              ;$F8
("SBC",     (3,3,3,3,3,3), (OP_ABS_Y)),         # SBC $3141,Y      ;F9
("PLX",     (1,1,1,1,1,0), (OP_NONE)),          # PLX              ;FA
("XCE",     (1,1,1,1,0,0), (OP_NONE)),          # XCE              ;FB
("JSR",     (3,3,3,3,0,0), (OP_ABS_X_IND)),     # JSR ($8085,X)    ;FC
("SBC",     (3,3,3,3,3,3), (OP_ABS_X)),         # SBC $9980,X      ;FD
("INC",     (3,3,3,3,3,3), (OP_ABS_X)),         # INC $9900,X      ;FE
("SBC",     (4,4,4,4,0,0), (OP_ABS_X_L))        # SBC $123456,X    ;$FF
)


class Memory:
# Super-cheesy : done for convenience instead of efficiency as a temporary function for testing and development only
    def __init__(self):
        self.m = bytearray(65536)
        self.sa = 0xFFFF
        self.enda = 0x0

    def write(self, addr16, data):
        if (addr16 < self.sa):
            self.sa = addr16
        if ((addr16 + len(data) - 1) > self.enda):
            self.enda = addr16 + len(data) - 1
        self.m[addr16:] = data
        return
    
    def read(self, addr, n):
        try:
            return self.m[addr:addr+n]
        except:
            return b''

    def readall(self):
        print("%04X:%04X" % (self.sa, self.enda))
        return self.sa, self.m[self.sa:self.enda+1]

    def clear(self):
        self.sa = 0xFFFF
        self.enda = 0x0
        self.m = bytearray(65536)

    def load_srec(self, srec_fn):
        global mem
        mem.clear()
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
                barry = str_to_bytes(datastr)
                mem.write(addr, barry)
                #dump_hex(addr, barry)
                #self.write_mem(addr, barry)
            elif rectype == 'S2':
                ads = li[4:10]
                nstr = li[2:4]
                n = int(nstr, 16) - 4   # Subtract checksum and 3 address bytes
                # print("Rec(S2) - 24 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%06X" % addr)
                datastr = li[10:-2]
                # print(len(datastr), datastr)
                barry = str_to_bytes(datastr)
                #dump_hex(addr, barry)
                mem.write(addr, barry)
                #self.write_mem(addr, barry)
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
                barry = str_to_bytes(datastr)
                #dump_hex(addr, barry)
                mem.write(addr, barry)
                #self.write_mem(addr, barry)
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

def to_hex(b):
    return int.from_bytes(b, "little")

        
class Disasm:
    # Mode = 0 : Native, 16 bit M, 16 bit X
    # Mode = 1 : Native, 16 bit M, 8 bit X
    # Mode = 2 : Native, 8 bit M, 16 bit X
    # Mode = 3 : Native, 8 bit M, 8 bit X or emulation mode 
    # Mode = 4 : Real 65c02
    # Mode = 5 : Real NMON 6502
    #
    def __init__(self, mode, offset, m):
        self.offset = offset
        self.pc = offset
        self.enda = offset + len(m)
        self.m = m
        self.mode = mode
        self.ir = b''               # The "instruction register"
        self.ir_len = 0
        self.OPFIELD_LEN = 20       # How long to make the opcode string to line up text to the right
        # 13 characters covers the longest 65c816 opcode, determined experimentally 

    def set_mode(self, mode):
        self.mode = mode

    def disassemble_one(self):
        ost = ""
        i = self.pc - self.offset
        op_byte = m[i]
        op = opcode_table[op_byte][0]
        self.ir_len = opcode_table[op_byte][1][self.mode]
        op_admode = opcode_table[op_byte][2]
        self.ir = m[i:i+self.ir_len]      # Get all the bytes (may be useful for various print reasons)
        ost = op

        if self.ir_len == 4:
            val = to_hex(m[i+1:i+4])
        elif self.ir_len == 3:
            val = to_hex(m[i+1:i+3])
        elif self.ir_len == 2:
            val = to_hex(m[i+1:i+2])

        if op_admode == OP_A:
            ost += " A"
        elif op_admode == OP_IMM:
            if self.ir_len == 3:
                ost += " #$%04X " % val
            else:
                ost += " #$%02X " % val
        elif op_admode == OP_ABS:
            ost += " $%04X " % val
        elif op_admode == OP_ZP:
            ost += " $%02X " % val
        elif op_admode == OP_ABS_L:
            ost += " $%06X " % val
        elif op_admode == OP_REL:
            if (val > 0x7F):
                val = 0x100 - val
            val = self.pc + 2 - val
            ost += " $%04X " % val
        elif op_admode == OP_REL_L:
            if (val > 0x7FFF):
                val = 0x10000 - val
                val = self.pc + 3 - val
                ost += " $%04X " % val
        elif op_admode == OP_ZP_XI:
            ost += " ($%02X,X) " % val
    
        elif op_admode == OP_ZP_IY:
            ost += " ($%02X),Y " % val

        elif op_admode == OP_ZP_IND_L:
            ost += " [$%02X] " % val

        elif op_admode == OP_ZP_IND:
            ost += " ($%02X) " % val

        elif op_admode == OP_ZP_IY_L:
            ost += " [$%02X],Y " % val

        elif op_admode == OP_ZP_X:
            ost += " $%02X,X " % val

        elif op_admode == OP_ZP_Y:
            ost += " $%02X,Y " % val

        elif op_admode == OP_ABS_X:
            ost += " $%04X,X " % val

        elif op_admode == OP_ABS_X_L:
            ost += " $%06X,X " % val

        elif op_admode == OP_ABS_Y:
            ost += " $%04X,Y " % val

        elif op_admode == OP_SR:
            ost += " $%02X,S " % val

        elif op_admode == OP_SR_IY:
            ost += " ($%02X,S),Y " % val

        elif op_admode == OP_ABS_IND:
            ost += " ($%04X) " % val
        elif op_admode == OP_ABS_IND_L:
            ost += " [$%06X] " % val
 
        elif op_admode == OP_ABS_X_IND:
            ost += " ($%04X,X) " % val

        elif op_admode == OP_2OPS:
            ost += " %d,%d " % (to_hex(m[i+1:i+2]), to_hex(m[i+2:i+3]))

        if len(ost) < self.OPFIELD_LEN:
            ost += ' '*(self.OPFIELD_LEN - len(ost))
        self.pc += self.ir_len
        return ost

        
    def disassemble_all(self):
        while self.pc < self.enda:
            outstr = "%06X: " % self.pc
            ou = self.disassemble_one()
            for i in range(6):
                if i < self.ir_len:
                    outstr += "%02X " % to_hex(self.ir[i:i+1])
                else:
                    outstr += "   "
            outstr += ou
            outstr += "(FLAGS output goes here)"
            print(outstr)


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
        self.m_flag = False
        self.x_flag = False

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
        cmd = b'\x03ou = self.disassemble_one()' + sal + sah + sab
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
        
    def init_registers(self, regdata):
        cmd = b'\x06'+regdata
        return self.cmd_dialog(cmd)

    def resume(self):
        cmd = b'\x07'
        return self.cmd_dialog(cmd)


    def decode_flags(self, f, emode):
        s = "FLAGS(%02X)=" % f
        if f & 0x80:
            s+= OP_IMM
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
        
    def send_srec(self, srec_fn):
        global mem

        mem.clear()
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
                barry = str_to_bytes(datastr)
                mem.write(addr, barry)
                #dump_hex(addr, barry)
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
                barry = str_to_bytes(datastr)
                #dump_hex(addr, barry)
                mem.write(addr, barry)
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
                barry = str_to_bytes(datastr)
                #dump_hex(addr, barry)
                mem.write(addr, barry)
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

def str_to_bytes(s):
        outb = b''
        for i in range(0, len(s), 2):
            x = int(s[i:i+2], 16)
            outb += x.to_bytes(1, 'little')
        return outb
        
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
    mem = Memory()
    for op in range(256):
        oplen = opcode_table[op][1][0:4]
        ref = oplen[0]
        for i in range(1,4):
            if oplen[i] != ref:
                print("%02X: %s" % (op, oplen))
            
    # print(len(opcode_table))
    # M0X0
    mem.load_srec("allops_m0x0.s19")
    sa, m = mem.readall()
    dump_hex(sa, m)
    print("========== M0X0 ==========")
    x = Disasm(0x0, sa, m)
    x.disassemble_all()

    # M0X1
    mem.load_srec("allops_m0x1.s19")
    sa, m = mem.readall()
    dump_hex(sa, m)
    print("========== M0X1 ==========")
    x = Disasm(0x1, sa, m)
    x.disassemble_all()

    # M1X0
    mem.load_srec("allops_m1x0.s19")
    sa, m = mem.readall()
    dump_hex(sa, m)
    print("========== M1X0 ==========")
    x = Disasm(0x2, sa, m)
    x.disassemble_all()

    # M1X1
    mem.load_srec("allops_m1x1.s19")
    sa, m = mem.readall()
    dump_hex(sa, m)
    print("========== M1X1 ==========")
    x = Disasm(0x3, sa, m)
    x.disassemble_all()

    exit(0)
    pipe = CPU_Pipe(SER_PORT, 921600)
    context = b'\x00\x84\x00\0x00\0x00\0x00\0x7D\0x00\0xFD\0x01\0x00\0x00\0x00\0x20\0x00\0x00'
    pipe.jump_long(0x002000)   # Apply context
    for i in range(10):
        pipe.print_registers()
        pipe.resume()
    exit(0)
    srec_fn = "m0x0.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    #bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    #pipe.replace_breaks()

    srec_fn = "m0x1.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    #bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("Jumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    #pipe.replace_breaks()

    srec_fn = "m1x0.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    #bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    #pipe.replace_breaks()

    srec_fn = "m1x1.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    #bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    #pipe.replace_breaks()

    srec_fn = "dow.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    #bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    #pipe.replace_breaks()

    exit(0)
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
   
   
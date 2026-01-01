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
    
#// Each entry corresponds to an opcode value $00-$FF
#// Bits 2-0 hold the instruction length 1-4
#// If Bit 7 is set, then if M=0 then add one to instruction length in b2..b0
#// If Bit 6 is set, then if X=0 then add one to instruction length in b2..b0
#// If Bit 5 is set, then this instruction is NOT supported on a real 65c02
#// If Bit 4 is set, then this instruction is NOT supported on a real NMOS 6502
#// THUS:
#// entry = op_decode[op]
#// ilen = entry & 0x7F
#// if (entry & 0x80) and M_FLAG == 0:
#//     ilen += 1
#// elif (entry & 0x40) and X_FLAG == 0:
#//     ilen += 1

'''
op_decode = [
0x01, 0x02, 0x32, 0x32, 0x12, 0x02, 0x02, 0x32,     # $00-$07
0x01, 0x82, 0x01, 0x31, 0x13, 0x03, 0x03, 0x34,     # $08-$0F
0x02, 0x02, 0x12, 0x32, 0x12, 0x02, 0x02, 0x32,     # $10-$17
0x01, 0x03, 0x11, 0x31, 0x13, 0x03, 0x03, 0x34,     # $18-$1F
0x03, 0x02, 0x34, 0x32, 0x02, 0x02, 0x02, 0x32,     # $20-$27
0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $28-$2F
0x02, 0x02, 0x12, 0x32, 0x12, 0x02, 0x02, 0x32,     # $30-$37
0x01, 0x03, 0x11, 0x31, 0x13, 0x03, 0x03, 0x34,     # $38-$3F
0x01, 0x32, 0x32, 0x32, 0x33, 0x02, 0x02, 0x32,     # $40-$47
0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $48-$4F
0x02, 0x02, 0x12, 0x32, 0x33, 0x02, 0x02, 0x32,     # $50-$57
0x01, 0x03, 0x11, 0x31, 0x34, 0x03, 0x03, 0x34,     # $58-$5F
0x01, 0x02, 0x33, 0x32, 0x12, 0x02, 0x02, 0x32,     # $60-$67
0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $68-$6F
0x02, 0x02, 0x02, 0x32, 0x12, 0x02, 0x02, 0x32,     # $70-$77
0x01, 0x03, 0x11, 0x31, 0x13, 0x03, 0x03, 0x34,     # $78-$7F
0x12, 0x02, 0x33, 0x32, 0x02, 0x02, 0x02, 0x32,     # $80-$87
0x01, 0x92, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $88-$8F
0x02, 0x02, 0x12, 0x32, 0x02, 0x02, 0x02, 0x32,     # $90-$97
0x01, 0x03, 0x01, 0x31, 0x13, 0x03, 0x13, 0x34,     # $98-$9F
0x42, 0x02, 0x42, 0x32, 0x02, 0x02, 0x02, 0x32,     # $A0-$A7
0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $A8-$AF
0x02, 0x02, 0x12, 0x32, 0x02, 0x02, 0x02, 0x32,     # $B0-$B7
0x01, 0x03, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $B8-$BF
0x42, 0x02, 0x32, 0x32, 0x02, 0x02, 0x02, 0x32,     # $C0-$C7
0x01, 0x82, 0x01, 0x11, 0x03, 0x03, 0x03, 0x34,     # $C8-$CF
0x02, 0x02, 0x12, 0x32, 0x32, 0x02, 0x02, 0x32,     # $D0-$D7
0x01, 0x03, 0x11, 0x11, 0x33, 0x03, 0x03, 0x34,     # $D8-$DF
0x42, 0x02, 0x32, 0x32, 0x02, 0x02, 0x02, 0x32,     # $E0-$E7
0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $E8-$EF
0x02, 0x02, 0x12, 0x32, 0x33, 0x02, 0x02, 0x32,     # $F0-$F7
0x01, 0x03, 0x11, 0x31, 0x33, 0x03, 0x03, 0x34      # $F8-$FF
]
'''


opcode_table = (
# {"MNEMONIC", ADDRMODE, ENCODING)
# ENCODING FIELD:
#// Each entry corresponds to an opcode value $00-$FF
#// Bits 2-0 hold the instruction length 1-4
#// If Bit 7 is set, then if M=0 then add one to instruction length in b2..b0
#// If Bit 6 is set, then if X=0 then add one to instruction length in b2..b0
#// If Bit 5 is set, then this instruction is NOT supported on a real 65c02
#// If Bit 4 is set, then this instruction is NOT supported on a real NMOS 6502
#// THUS:
#// entry = op_decode[op]
#// ilen = entry & 0x7F
#// if (entry & 0x80) and M_FLAG == 0:
#//     ilen += 1
#// elif (entry & 0x40) and X_FLAG == 0:
#//     ilen += 1


# $00
#0x01, 0x02, 0x32, 0x32, 0x12, 0x02, 0x02, 0x32,     # $00-$07
("BRK",  OP_NONE,       0x01),      # BRK               ;$00
("ORA",  OP_ZP_XI,      0x02),      # ORA ($30,X)       ;$01
("COP",  OP_IMM,        0x32),      # COP #10           ;$02
("ORA",  OP_SR,         0x32),      # ORA 1,S           ;$03 
("TSB",  OP_ZP,         0x12),      # TSB $30           ;$04  
("ORA",  OP_ZP,         0x02),      # ORA $42           ;$05
("ASL",  OP_ZP,         0x02),      # ASL $22           ;$06
("ORA",  OP_ZP_IND_L,   0x32),      # ORA [$33]         ;$07
# 08
#0x01, 0x82, 0x01, 0x31, 0x13, 0x03, 0x03, 0x34,     # $08-$0F
("PHP",  OP_NONE,       0x01),      # PHP              ;$08
("ORA",  OP_IMM,        0x82),      # ORA #$42         ;$09
("ASL",  OP_A,          0x01),      # ASL A            ;$0A
("PHD",  OP_NONE,       0x31),      # PHD              ;$0B
("TSB",  OP_ABS,        0x13),      # TSB $0300        ;$0C
("ORA",  OP_ABS,        0x03),      # ORA $4242        ;$0D
("ASL",  OP_ABS,        0x03),      # ASL $5150        ;$0E
("ORA",  OP_ABS_L,      0x34),      # ORA $123124      ;$0F
# 10
#0x02, 0x02, 0x12, 0x32, 0x12, 0x02, 0x02, 0x32,     # $10-$17
("BPL",  OP_REL,        0x02),      # BPL HERE         ;$10
("ORA",  OP_ZP_IY,      0x02),      # ORA ($42),y      ;$11
("ORA",  OP_ZP_IND,     0x12),      # ORA ($42)        ;$12
("ORA",  OP_SR_IY,      0x32),      # ORA (1,S),Y      ;$13
("TRB",  OP_ZP,         0x12),      # TRB $19          ;$14
("ORA",  OP_ZP_X,       0x02),      # ORA $42,X        ;$15
("ASL",  OP_ZP_X,       0x02),      # ASL $43,X        ;$16
("ORA",  OP_ZP_IY_L,    0x32),      # ORA [$12],Y      ;$17
# 18
#0x01, 0x03, 0x11, 0x31, 0x13, 0x03, 0x03, 0x34,     # $18-$1F
("CLC",  OP_NONE,       0x01),      # CLC              ;$18
("ORA",  OP_ABS_Y,      0x03),      # ORA $2A04,Y      ;$19
("INC",  OP_A,          0x11),      # INC A            ;$1A
("TCS",  OP_NONE,       0x31),      # TCS              ;1B
("TRB",  OP_ABS,        0x13),      # TRB $1234        ;1C
("ORA",  OP_ABS_X,      0x03),      # ORA $0300,X      ;1D
("ASL",  OP_ABS_X,      0x03),      # ASL $4242,X      ;1E
("ORA",  OP_ABS_X_L,    0x34),      # ORA $402011,X    ;1F
# 20
#0x03, 0x02, 0x34, 0x32, 0x02, 0x02, 0x02, 0x32,     # $20-$27
("JSR",  OP_ABS,        0x03),      # JSR HERE         ;$20 
("AND",  OP_ZP_XI,      0x02),      # AND ($69,X)      ;21
("JSL",  OP_ABS_L,      0x34),      # JSL HERE         ;22
("AND",  OP_SR,         0x32),      # AND 1,S          ;23
("BIT",  OP_ZP,         0x02),      # BIT $20          ;24
("AND",  OP_ZP,         0x02),      # AND $30          ;25
("ROL",  OP_ZP,         0x02),      # ROL $12          ;26
("AND",  OP_ZP_IND_L,   0x32),      # AND [$10]        ;27
# 28
#0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $28-$2F
("PLP",  OP_NONE,       0x01),      # PLP              ;$28
("AND",  OP_IMM,        0x82),      # AND #$FE         ;$29
("ROL",  OP_A,          0x01),      # ROL A            ;2A
("PLD",  OP_NONE,       0x31),      # PLD              ;2B
("BIT",  OP_ABS,        0x03),      # BIT $0BCD        ;2C
("AND",  OP_ABS,        0x03),      # AND $0DEF        ;2D
("ROL",  OP_ABS,        0x03),      # ROL $5150        ;2E
("AND",  OP_ABS_L,      0x34),      # AND $123456      ;2F
#30 
#0x02, 0x02, 0x12, 0x32, 0x12, 0x02, 0x02, 0x32,     # $30-$37    
("BMI",  OP_REL,        0x02),      # BMI HERE         ;$30
("AND",  OP_ZP_IY,      0x02),      # AND ($20),Y      ;31
("AND",  OP_ZP_IND,     0x12),      # AND ($20)        ;32
("AND",  OP_SR_IY,      0x32),      # AND (1,S),Y      ;33
("BIT",  OP_ZP_X,       0x12),      # BIT $20,X        ;34
("AND",  OP_ZP_X,       0x02),      # AND $21,X        ;35
("ROL",  OP_ZP_X,       0x02),      # ROL $69,X        ;36
("AND",  OP_ZP_IY_L,    0x32),      # AND [$30],Y      ;37
# 38
#0x01, 0x03, 0x11, 0x31, 0x13, 0x03, 0x03, 0x34,     # $38-$3F
("SEC",  OP_NONE,       0x01),      # SEC              ;$38
("AND",  OP_ABS_Y,      0x03),      # AND $2A04,Y      ;39
("DEC",  OP_A,          0x11),      # DEC A            ;3A
("TSC",  OP_NONE,       0x31),      # TSC              ;3B
("BIT",  OP_ABS_X,      0x13),      # BIT $ABCD,X      ;3C
("AND",  OP_ABS_X,      0x03),      # AND $1234,X      ;3D
("ROL",  OP_ABS_X,      0x03),      # ROL $5150,X      ;3E
("AND",  OP_ABS_X_L,    0x34),      # AND $123456,X    ;3F
# 40
#0x01, 0x32, 0x32, 0x32, 0x33, 0x02, 0x02, 0x32,     # $40-$47
("RTI",  OP_NONE,       0x01),      # RTI              ;$40
("EOR",  OP_ZP_XI,      0x32),      # EOR ($22,X)      ;41
("WDM",  OP_IMM,        0x32),      # WDM #$AA         ;42
("EOR",  OP_SR,         0x32),      # EOR 1,S          ;43
("MVP",  OP_2OPS,       0x33),      # MVP 1,2          ;44
("EOR",  OP_ZP,         0x02),      # EOR $12          ;45
("LSR",  OP_ZP,         0x02),      # LSR $33          ;46 
("EOR",  OP_ZP_IND_L,   0x32),      # EOR [$27]        ;47
# 48
#0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $48-$4F
("PHA",  OP_NONE,       0x01),      # PHA              ;$48
("EOR",  OP_IMM,        0x82),      # EOR #$FF         ;49
("LSR",  OP_A,          0x01),      # LSR A            ;4A
("PHK",  OP_NONE,       0x31),      # PHK              ;4B
("JMP",  OP_ABS,        0x03),      # JMP $2A04        ;4C
("EOR",  OP_ABS,        0x03),      # EOR $ABCD        ;4D
("LSR",  OP_ABS,        0x03),      # LSR $2525        ;4E
("EOR",  OP_ABS_L,      0x34),      # EOR $123456      ;4F
# 50
#0x02, 0x02, 0x12, 0x32, 0x33, 0x02, 0x02, 0x32,     # $50-$57
("BVC",  OP_REL,        0x02),      # BVC THERE        ;$50
("EOR",  OP_ZP_IY,      0x02),      # EOR ($21),Y      ;51
("EOR",  OP_ZP_IND,     0x12),      # EOR ($25)        ;52
("EOR",  OP_SR_IY,      0x32),      # EOR (1,S),Y      ;53
("MVN",  OP_2OPS,       0x33),      # MVN 3,4          ;54
("EOR",  OP_ZP_X,       0x02),      # EOR $19,X        ;55
("LSR",  OP_ZP_X,       0x02),      # LSR $18,X        ;56
("EOR",  OP_ZP_IY_L,    0x32),      # EOR [$42],Y      ;57
# 58
#0x01, 0x03, 0x11, 0x31, 0x34, 0x03, 0x03, 0x34,     # $58-$5F
("CLI",  OP_NONE,       0x01),      # CLI              ;$58
("EOR",  OP_ABS_Y,      0x03),      # EOR $0200,Y      ;59
("PHY",  OP_NONE,       0x11),      # PHY              ;5A
("TCD",  OP_NONE,       0x31),      # TCD              ;5B
("JML",  OP_ABS_L,      0x34),      # JML $123456      ;5C
("EOR",  OP_ABS_X,      0x03),      # EOR $1234,X      ;5D
("LSR",  OP_ABS_X,      0x03),      # LSR $4231,X      ;5E
("EOR",  OP_ABS_X_L,    0x34),      # EOR $123456,X    ;5F
# 60
#0x01, 0x02, 0x33, 0x32, 0x12, 0x02, 0x02, 0x32,     # $60-$67
("RTS",  OP_NONE,       0x01),      # RTS              ;$60
("ADC",  OP_ZP_XI,      0x02),      # ADC ($50,X)      ;61
("PER",  OP_REL,        0x33),      # PER THERE        ;62
("ADC",  OP_SR,         0x32),      # ADC 1,S          ;63
("STZ",  OP_ZP,         0x12),      # STZ $30          ;64
("ADC",  OP_ZP,         0x02),      # ADC $30          ;65
("ROR",  OP_ZP,         0x02),      # ROR $59          ;66
("ADC",  OP_ZP_IND_L,   0x32),      # ADC [$89]        ;67
# 68
#0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $68-$6F
("PLA",  OP_NONE,       0x01),      # PLA              ;$68
("ADC",  OP_IMM,        0x82),      # ADC #$23         ;69
("ROR",  OP_A,          0x01),      # ROR A            ;6A
("RTL",  OP_NONE,       0x31),      # RTL              ;6B
("JMP",  OP_ABS_IND,    0x03),      # JMP ($FFFC)      ;6C
("ADC",  OP_ABS,        0x03),      # ADC $1234        ;6D
("ROR",  OP_ABS,        0x03),      # ROR $2345        ;6E
("ADC",  OP_ABS_L,      0x34),      # ADC $123456      ;6F
# 70
#0x02, 0x02, 0x02, 0x32, 0x12, 0x02, 0x02, 0x32,     # $70-$77
("BVS",  OP_REL,        0x02),      # BVS THERE        ;$70
("ADC",  OP_ZP_IY,      0x02),      # ADC ($21),Y      ;71
("ADC",  OP_ZP_IND,     0x02),      # ADC ($21)        ;72
("ADC",  OP_SR_IY,      0x32),      # ADC (1,S),Y      ;73
("STZ",  OP_ZP_X,       0x12),      # STZ $12,X        ;74
("ADC",  OP_ZP_X,       0x02),      # ADC $11,X        ;75
("ROR",  OP_ZP_X,       0x02),      # ROR $55,X        ;76
("ADC",  OP_ZP_IY_L,    0x32),      # ADC [$42],Y      ;77
# 78
#0x01, 0x03, 0x11, 0x31, 0x13, 0x03, 0x03, 0x34,     # $78-$7F
("SEI",  OP_NONE,       0x01),      # SEI              ;78
("ADC",  OP_ABS_Y,      0x03),      # ADC $0100,Y      ;79
("PLY",  OP_NONE,       0x11),      # PLY              ;7A
("TDC",  OP_NONE,       0x31),      # TDC              ;7B
("JMP",  OP_ABS_X_IND,  0x13),      # JMP ($8080,X)    ;7C
("ADC",  OP_ABS_X,      0x03),      # ADC $1234,X      ;7D
("ROR",  OP_ABS_X,      0x03),      # ROR $3456,X      ;7E
("ADC",  OP_ABS_X_L,    0x34),      # ADC $123456,X    ;7F
# 80
#0x12, 0x02, 0x33, 0x32, 0x02, 0x02, 0x02, 0x32,     # $80-$87
("BRA",  OP_REL,        0x12),      # BRA T01          ;$80
("STA",  OP_ZP_XI,      0x02),      # STA ($00,X)      ;81
("BRL",  OP_REL_L,      0x33),      # BRL T01          ;82
("STA",  OP_SR,         0x32),      # STA 1,S          ;83
("STY",  OP_ZP,         0x02),      # STY $21          ;84
("STA",  OP_ZP,         0x02),      # STA $64          ;85
("STX",  OP_ZP,         0x02),      # STX $99          ;86
("STA",  OP_ZP_IND_L,   0x32),      # STA [$55]        ;87
# 88
#0x01, 0x92, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $88-$8F
("DEY",  OP_NONE,       0x01),      # DEY              ;88
("BIT",  OP_IMM,        0x92),      # BIT #$31         ;89
("TXA",  OP_NONE,       0x01),      # TXA              ;8A
("PHB",  OP_NONE,       0x31),      # PHB              ;8B
("STY",  OP_ABS,        0x03),      # STY $5150        ;8C
("STA",  OP_ABS,        0x03),      # STA $1234        ;8D
("STX",  OP_ABS,        0x03),      # STX $2345        ;8E
("STA",  OP_ABS_L,      0x34),      # STA $123456      ;8F
# 90
#0x02, 0x02, 0x12, 0x32, 0x02, 0x02, 0x02, 0x32,     # $90-$97
("BCC",  OP_REL,        0x02),      # BCC T01          ;$90
("STA",  OP_ZP_IY,      0x02),      # STA ($22),Y      ;91
("STA",  OP_ZP_IND,     0x12),      # STA ($AA)        ;92
("STA",  OP_SR_IY,      0x32),      # STA (1,S),Y      ;93
("STY",  OP_ZP_X,       0x02),      # STY $55,X        ;94
("STA",  OP_ZP_X,       0x02),      # STA $55,X        ;95
("STX",  OP_ZP_Y,       0x02),      # STX $55,Y        ;96
("STA",  OP_ZP_IY_L,    0x32),      # STA [$42],Y      ;97
# 98
#0x01, 0x03, 0x01, 0x31, 0x13, 0x03, 0x13, 0x34,     # $98-$9F
("TYA",  OP_NONE,       0x01),      # TYA              ;$98
("STA",  OP_ABS_Y,      0x03),      # STA $1200,Y      ;99
("TXS",  OP_NONE,       0x01),      # TXS              ;9A
("TXY",  OP_NONE,       0x31),      # TXY              ;9B
("STZ",  OP_ABS,        0x13),      # STZ $5150        ;9C
("STA",  OP_ABS_X,      0x03),      # STA $0500,X      ;9D
("STZ",  OP_ABS_X,      0x13),      # STZ $5555,X      ;9E
("STA",  OP_ABS_X_L,    0x34),      # STA $123456,X    ;9F
# A0
#0x42, 0x02, 0x42, 0x32, 0x02, 0x02, 0x02, 0x32,     # $A0-$A7
("LDY",  OP_IMM,        0x42),      # LDY #$44         ;$A0
("LDA",  OP_ZP_XI,      0x02),      # LDA ($27,X)      ;A1
("LDX",  OP_IMM,        0x42),      # LDX #$24         ;A2
("LDA",  OP_SR,         0x32),      # LDA 5,S          ;A3
("LDY",  OP_ZP,         0x02),      # LDY $55          ;A4
("LDA",  OP_ZP,         0x02),      # LDA $68          ;A5
("LDX",  OP_ZP,         0x02),      # LDX $88          ;A6
("LDA",  OP_ZP_IND_L,   0x32),      # LDA [$20]        ;A7
# A8
#0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $A8-$AF
("TAY",  OP_NONE,       0x01),      # TAY              ;$A8
("LDA",  OP_IMM,        0x82),      # LDA #$AB         ;$A9
("TAX",  OP_NONE,       0x01),      # TAX              ;AA
("PLB",  OP_NONE,       0x31),      # PLB              ;AB
("LDY",  OP_ABS,        0x03),      # LDY $1234        ;AC
("LDA",  OP_ABS,        0x03),      # LDA $1234        ;AD
("LDX",  OP_ABS,        0x03),      # LDX $1234        ;AE
("LDA",  OP_ABS_L,      0x34),      # LDX $123456        ;AF
# B0
#0x02, 0x02, 0x12, 0x32, 0x02, 0x02, 0x02, 0x32,     # $B0-$B7
("BCS",  OP_REL,        0x02),      # BCS T02          ;$B0  
("LDA",  OP_ZP_IY,      0x02),      # LDA ($88),Y      ;B1
("LDA",  OP_ZP_IND,     0x12),      # LDA ($9A)        ;B2
("LDA",  OP_SR_IY,      0x32),      # LDA (3,S),Y      ;B3
("LDY",  OP_ZP_X,       0x02),      # LDY $50,X        ;B4
("LDA",  OP_ZP_X,       0x02),      # LDA $50,X        ;B5
("LDX",  OP_ZP_Y,       0x02),      # LDX $50,Y        ;B6
("LDA",  OP_ZP_IY_L,    0x32),      # LDA [$40],Y      ;B7
# B8
#0x01, 0x03, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $B8-$BF
("CLV",  OP_NONE,       0x01),      # CLV              ;$B8
("LDA",  OP_ABS_Y,      0x03),      # LDA $2525,Y      ;B9
("TSX",  OP_NONE,       0x01),      # TSX              ;BA
("TYX",  OP_NONE,       0x31),      # TYX              ;BB
("LDY",  OP_ABS_X,      0x03),      # LDY $0211,X      ;BC
("LDA",  OP_ABS_X,      0x03),      # LDA $2101,X      ;BD
("LDX",  OP_ABS_Y,      0x03),      # LDX $2012,Y      ;BE
("LDA",  OP_ABS_X_L,    0x34),      # LDA $123456,X    ;BF
# C0
#0x42, 0x02, 0x32, 0x32, 0x02, 0x02, 0x02, 0x32,     # $C0-$C7
("CPY",  OP_IMM,        0x42),      # CPY #$42         ;$C0
("CMP",  OP_ZP_XI,      0x02),      # CMP ($50,X)      ;C1
("REP",  OP_IMM,        0x32),      # REP #$30         ;C2
("CMP",  OP_SR,         0x32),      # CMP 1,S          ;C3
("CPY",  OP_ZP,         0x02),      # CPY $22          ;C4
("CMP",  OP_ZP,         0x02),      # CMP $33          ;C5
("DEC",  OP_ZP,         0x02),      # DEC $44          ;C6
("CMP",  OP_ZP_IND_L,   0x32),      # CMP [$42]        ;C7
# C8
#0x01, 0x82, 0x01, 0x11, 0x03, 0x03, 0x03, 0x34,     # $C8-$CF
("INY",  OP_NONE,       0x01),      # INY              ; $C8
("CMP",  OP_IMM,        0x82),      # CMP #$51         ;C9
("DEX",  OP_NONE,       0x01),      # DEX              ;CA
("WAI",  OP_NONE,       0x11),      # WAI              ;CB
("CPY",  OP_ABS,        0x03),      # CPY $4242        ;CC
("CMP",  OP_ABS,        0x03),      # CMP $4141        ;CD
("DEC",  OP_ABS,        0x03),      # DEC $2525        ;CE
("CMP",  OP_ABS_L,      0x34),      # CMP $125050      ;CF
# D0
#0x02, 0x02, 0x12, 0x32, 0x32, 0x02, 0x02, 0x32,     # $D0-$D7
("BNE",  OP_REL,        0x02),      # BNE T03          ;$D0
("CMP",  OP_ZP_IY,      0x02),      # CMP ($00),Y      ;D1
("CMP",  OP_ZP_IND,     0x12),      # CMP ($01)        ;D2
("CMP",  OP_SR_IY,      0x32),      # CMP (1,S),Y      ;D3
("PEI",  OP_ZP,         0x32),      # PEI $31          ;D4
("CMP",  OP_ZP_X,       0x02),      # CMP $99,X        ;D5
("DEC",  OP_ZP_X,       0x02),      # DEC $AA,X        ;D6
("CMP",  OP_ZP_IY_L,    0x32),      # CMP [$33],Y      ;D7
# D8
#0x01, 0x03, 0x11, 0x11, 0x33, 0x03, 0x03, 0x34,     # $D8-$DF
("CLD",  OP_NONE,       0x01),      # CLD              ;$D8
("CMP",  OP_ABS_Y,      0x03),      # CMP $3124,Y      ;D9
("PHX",  OP_NONE,       0x11),      # PHX              ;DA
("STP",  OP_NONE,       0x11),      # STP              ;DB
("JML",  OP_ABS_IND_L,  0x33),      # JML [$1234]      ;DC
("CMP",  OP_ABS_X,      0x03),      # CMP $5000,X      ;DD
("DEC",  OP_ABS_X,      0x03),      # DEC $5150,X      ;DE
("CMP",  OP_ABS_X_L,    0x34),      # CMP $123456,X    ;DF
# E0
#0x42, 0x02, 0x32, 0x32, 0x02, 0x02, 0x02, 0x32,     # $E0-$E7
("CPX",  OP_IMM,        0x42),      # CPX #$FF         ;$E0
("SBC",  OP_ZP_XI,      0x02),      # SBC ($01,X)      ;E1
("SEP",  OP_IMM,        0x32),      # SEP #$00         ;E2
("SBC",  OP_SR,         0x32),      # SBC 1,S          ;E3
("CPX",  OP_ZP,         0x02),      # CPX $33          ;E4
("SBC",  OP_ZP,         0x02),      # SBC $99          ;E5
("INC",  OP_ZP,         0x02),      # INC $28          ;E6
("SBC",  OP_ZP_IND_L,   0x32),      # SBC [$42]        ;E7
# E8
#0x01, 0x82, 0x01, 0x31, 0x03, 0x03, 0x03, 0x34,     # $E8-$EF
("INX",  OP_NONE,       0x01),      # INX              ;$E8
("SBC",  OP_IMM,        0x82),      # SBC #$12         ;E9
("NOP",  OP_NONE,       0x01),      # NOP              ;EA
("XBA",  OP_NONE,       0x31),      # XBA              ;EB
("INC",  OP_ABS,        0x03),      # CPX $3124        ;EC
("SBC",  OP_ABS,        0x03),      # SBC $5101        ;ED
("INC",  OP_ABS,        0x03),      # INC $2222        ;EE
("SBC",  OP_ABS_L,      0x34),      # SBC $123456      ;EF
# F0
#0x02, 0x02, 0x12, 0x32, 0x33, 0x02, 0x02, 0x32,     # $F0-$F7
("BEQ",  OP_REL,        0x02),      # BEQ T03          ;$F0
("SBC",  OP_ZP_IY,      0x02),      # SBC ($94),Y      ;F1
("SBC",  OP_ZP_IND,     0x12),      # SBC ($88)        ;F2
("SBC",  OP_SR_IY,      0x32),      # SBC (4,S),Y      ;F3
("PEA",  OP_IMM,        0x33),      # PEA T03          ;F4
("SBC",  OP_ZP_X,       0x02),      # SBC $44,X        ;F5
("INC",  OP_ZP_X,       0x02),      # INC $44,X        ;F6
("SBC",  OP_ZP_IY_L,    0x32),      # SBC [$42],Y      ;F7
# F8
#0x01, 0x03, 0x11, 0x31, 0x33, 0x03, 0x03, 0x34      # $F8-$FF
("SED",  OP_NONE,       0x01),      # SED              ;$F8
("SBC",  OP_ABS_Y,      0x03),      # SBC $3141,Y      ;F9
("PLX",  OP_NONE,       0x11),      # PLX              ;FA
("XCE",  OP_NONE,       0x31),      # XCE              ;FB
("JSR",  OP_ABS_X_IND,  0x33),      # JSR ($8085,X)    ;FC
("SBC",  OP_ABS_X,      0x03),      # SBC $9980,X      ;FD
("INC",  OP_ABS_X,      0x03),      # INC $9900,X      ;FE
("SBC",  OP_ABS_X_L,    0x04)       # SBC $123456,X    ;$FF
)


def get_mnemonic(opcode):
    return opcode_table[opcode][0]

def get_ilen(opcode, mode):
    lut = opcode_table[opcode][2]
    if (mode == CPU_MODE_NMOS_6502) and (lut & 0x10):
        return 0    # Instruction not supported on an actual NMOS 6502
    if (mode == CPU_MODE_CMOS_6502) and (lut & 0x20):
        return 0    # Instruction not supported on an actual 65c02 chip (vs. 65816 in E mode)
    irl = lut & 0x7
    if lut & 0x80:
        if (mode & 2) == 0:
            irl += 1
    elif lut & 0x40:
        if (mode & 1) == 0:
            irl += 1
    return irl
def get_address_mode(opcode):
    return opcode_table[opcode][1]

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

        op = get_mnemonic(op_byte)
        op_admode = get_address_mode(op_byte)
        self.ir_len = get_ilen(op_byte, self.mode) # Get instruction length by op-code and mode flags        
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
    i = 0
    print("op_tbl opcode_table = {")
    for op in opcode_table:
        s = "{\""
        s += op[0]
        s += "\","
        s += "0x%02X" % op[2]
        s +=  ","
        s += "%d" % op[1]
        s += "},"
        sl = len(s)
        if 20 - sl > 0:
            s += ' '*(20 - sl)
        s += "//"
        s += "$%02X" % i
        i += 1
        print(s)
    print("}")
    exit(0)
    '''
    opextra = list()
    for op in range(256):
        oplen = opcode_table[op][1][0:4]
        c02_supported = opcode_table[op][1][4] != 0
        nmos_supported = opcode_table[op][1][5] != 0
        ilen = 5
        diff = 0
        tblcode = 0
        ref = oplen[0]
        for i in range(4):
            v = oplen[i]
            if (v < ilen):
                ilen = v
            if (v != ref):
                diff = 1
        fs = " "
        tblcode = ilen
        if diff:
            if oplen[0] != oplen[1]:
                fs = "X"
                tblcode += 0x40
            else:
                fs = "M"
                tblcode += 0x80

        if c02_supported == False:
            tblcode += 0x20
        if nmos_supported == False:
            tblcode += 0x10
        print("%02X: %s --> %d %s %s %s\t[%02X]" % (op, oplen, ilen, fs, str(c02_supported), str(nmos_supported), tblcode))
        opextra.append(tblcode)
    print("[", end="")
    for i in range(256):
        print("0x%02X, " % opextra[i], end="")
        if i % 8 == 7:
            print("    # $%02X-$%02X" % ((i-7), i))
    print("]")
    print(len(opextra))
    exit(0)
    '''
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
   
   

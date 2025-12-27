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


#define MAX_OPS_LEN (3)
typedef enum
{
        OP_NONE = 0, OP_A, OP_IMM, OP_ABS, OP_ABS_L, OP_ABS_IND, OP_ABS_IND_L,
        OP_ABS_X, OP_ABS_Y, OP_ABS_X_L, OP_ABS_X_IND, OP_ZP,
        OP_ZP_IND, OP_ZP_IND_L, OP_ZP_X, OP_ZP_Y, OP_ZP_XI,
        OP_ZP_IY, OP_ZP_IY_L, OP_REL = 19, OP_REL_L, OP_SR, OP_SR_IY,OP_2OPS
} address_mode_t;


typedef struct op_tbl
{
    char ops[MAX_OPS_LEN+1];	// Hold full opcode plus null
    uint8_t sizeinfo;
    address_mode_t adm;

} op_tbl;

const uint8_t LEN1 = 0x01;
const uint8_t LEN2 = 0x02;
const uint8_t LEN3 = 0x03;
const uint8_t LEN4 = 0x04;
const uint8_t M_ADDS = 0x80;	// If M flag is 0, add 1 to ilen
const uint8_t X_ADDS = 0x40;	// If X = 0, then add 1 to inst len
const uint8_t NOT_65C02 = 0x20;		// Instruction not supported 65c02
const uint8_t NOT_6502 = 0x10;		// Instruction not supported NMOS


op_tbl opcode_table[] = {
{"BRK",	LEN1,	OP_NONE},     //$00
{"ORA",	LEN2,	OP_ZP_XI},    //$01
{"COP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_IMM},     //$02
{"ORA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$03
{"TSB",	(LEN2 | NOT_6502),	OP_ZP},    //$04
{"ORA",	LEN2,	OP_ZP},    //$05
{"ASL",	LEN2,	OP_ZP},    //$06
{"ORA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$07
{"PHP",	LEN1,	OP_NONE},     //$08
{"ORA",	(LEN2 | M_ADDS),	OP_IMM},     //$09
{"ASL",	LEN1,	OP_A},     //$0A
{"PHD",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$0B
{"TSB",	(LEN3 | NOT_6502),	OP_ABS},     //$0C
{"ORA",	LEN3,	OP_ABS},     //$0D
{"ASL",	LEN3,	OP_ABS},     //$0E
{"ORA",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$0F
{"BPL",	LEN2,	OP_REL},    //$10
{"ORA",	LEN2,	OP_ZP_IY},    //$11
{"ORA",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$12
{"ORA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$13
{"TRB",	(LEN2 | NOT_6502),	OP_ZP},    //$14
{"ORA",	LEN2,	OP_ZP_X},    //$15
{"ASL",	LEN2,	OP_ZP_X},    //$16
{"ORA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$17
{"CLC",	LEN1,	OP_NONE},     //$18
{"ORA",	LEN3,	OP_ABS_Y},     //$19
{"INC",	(LEN1 | NOT_6502),	OP_A},     //$1A
{"TCS",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$1B
{"TRB",	(LEN3 | NOT_6502),	OP_ABS},     //$1C
{"ORA",	LEN3,	OP_ABS_X},     //$1D
{"ASL",	LEN3,	OP_ABS_X},     //$1E
{"ORA",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_X_L},     //$1F
{"JSR",	LEN3,	OP_ABS},     //$20
{"AND",	LEN2,	OP_ZP_XI},    //$21
{"JSL",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$22
{"AND",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$23
{"BIT",	LEN2,	OP_ZP},    //$24
{"AND",	LEN2,	OP_ZP},    //$25
{"ROL",	LEN2,	OP_ZP},    //$26
{"AND",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$27
{"PLP",	LEN1,	OP_NONE},     //$28
{"AND",	(LEN2 | M_ADDS),	OP_IMM},     //$29
{"ROL",	LEN1,	OP_A},     //$2A
{"PLD",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$2B
{"BIT",	LEN3,	OP_ABS},     //$2C
{"AND",	LEN3,	OP_ABS},     //$2D
{"ROL",	LEN3,	OP_ABS},     //$2E
{"AND",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$2F
{"BMI",	LEN2,	OP_REL},    //$30
{"AND",	LEN2,	OP_ZP_IY},    //$31
{"AND",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$32
{"AND",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$33
{"BIT",	(LEN2 | NOT_6502),	OP_ZP_X},    //$34
{"AND",	LEN2,	OP_ZP_X},    //$35
{"ROL",	LEN2,	OP_ZP_X},    //$36
{"AND",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$37
{"SEC",	LEN1,	OP_NONE},     //$38
{"AND",	LEN3,	OP_ABS_Y},     //$39
{"DEC",	(LEN1 | NOT_6502),	OP_A},     //$3A
{"TSC",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$3B
{"BIT",	(LEN3 | NOT_6502),	OP_ABS_X},     //$3C
{"AND",	LEN3,	OP_ABS_X},     //$3D
{"ROL",	LEN3,	OP_ABS_X},     //$3E
{"AND",	(LEN4 | NOT_6502),	OP_ABS_X_L},     //$3F
{"RTI",	LEN1,	OP_NONE},     //$40
{"EOR",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_XI},    //$41
{"WDM",	(LEN2 | NOT_65C02 | NOT_6502),	OP_IMM},     //$42
{"EOR",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$43
{"MVP",	(LEN3 | NOT_65C02 | NOT_6502),	OP_2OPS},    //$44
{"EOR",	LEN2,	OP_ZP},    //$45
{"LSR",	LEN2,	OP_ZP},    //$46
{"EOR",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$47
{"PHA",	LEN1,	OP_NONE},     //$48
{"EOR",	(LEN2 | M_ADDS),	OP_IMM},     //$49
{"LSR",	LEN1,	OP_A},     //$4A
{"PHK",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$4B
{"JMP",	LEN3,	OP_ABS},     //$4C
{"EOR",	LEN3,	OP_ABS},     //$4D
{"LSR",	LEN3,	OP_ABS},     //$4E
{"EOR",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$4F
{"BVC",	LEN2,	OP_REL},    //$50
{"EOR",	LEN2,	OP_ZP_IY},    //$51
{"EOR",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$52
{"EOR",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$53
{"MVN",	(LEN3 | NOT_65C02 | NOT_6502),	OP_2OPS},    //$54
{"EOR",	LEN2,	OP_ZP_X},    //$55
{"LSR",	LEN2,	OP_ZP_X},    //$56
{"EOR",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$57
{"CLI",	LEN1,	OP_NONE},     //$58
{"EOR",	LEN3,	OP_ABS_Y},     //$59
{"PHY",	(LEN1 | NOT_6502),	OP_NONE},     //$5A
{"TCD",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$5B
{"JML",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$5C
{"EOR",	LEN3,	OP_ABS_X},     //$5D
{"LSR",	LEN3,	OP_ABS_X},     //$5E
{"EOR",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_X_L},     //$5F
{"RTS",	LEN1,	OP_NONE},     //$60
{"ADC",	LEN2,	OP_ZP_XI},    //$61
{"PER",	(LEN3 | NOT_65C02 | NOT_6502),	OP_REL},    //$62
{"ADC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$63
{"STZ",	(LEN2 | NOT_6502),	OP_ZP},    //$64
{"ADC",	LEN2,	OP_ZP},    //$65
{"ROR",	LEN2,	OP_ZP},    //$66
{"ADC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$67
{"PLA",	LEN1,	OP_NONE},     //$68
{"ADC",	(LEN2 | M_ADDS),	OP_IMM},     //$69
{"ROR",	LEN1,	OP_A},     //$6A
{"RTL",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$6B
{"JMP",	LEN3,	OP_ABS_IND},     //$6C
{"ADC",	LEN3,	OP_ABS},     //$6D
{"ROR",	LEN3,	OP_ABS},     //$6E
{"ADC",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$6F
{"BVS",	LEN2,	OP_REL},    //$70
{"ADC",	LEN2,	OP_ZP_IY},    //$71
{"ADC",	LEN2,	OP_ZP_IND},    //$72
{"ADC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$73
{"STZ",	(LEN2 | NOT_6502),	OP_ZP_X},    //$74
{"ADC",	LEN2,	OP_ZP_X},    //$75
{"ROR",	LEN2,	OP_ZP_X},    //$76
{"ADC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$77
{"SEI",	LEN1,	OP_NONE},     //$78
{"ADC",	LEN3,	OP_ABS_Y},     //$79
{"PLY",	(LEN1 | NOT_6502),	OP_NONE},     //$7A
{"TDC",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$7B
{"JMP",	(LEN3 | NOT_6502),	OP_ABS_X_IND},    //$7C
{"ADC",	LEN3,	OP_ABS_X},     //$7D
{"ROR",	LEN3,	OP_ABS_X},     //$7E
{"ADC",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_X_L},     //$7F
{"BRA",	(LEN2 | NOT_6502),	OP_REL},    //$80
{"STA",	LEN2,	OP_ZP_XI},    //$81
{"BRL",	(LEN3 | NOT_65C02 | NOT_6502),	OP_REL_L},    //$82
{"STA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$83
{"STY",	LEN2,	OP_ZP},    //$84
{"STA",	LEN2,	OP_ZP},    //$85
{"STX",	LEN2,	OP_ZP},    //$86
{"STA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$87
{"DEY",	LEN1,	OP_NONE},     //$88
{"BIT",	0x92,	OP_IMM},     //$89
{"TXA",	LEN1,	OP_NONE},     //$8A
{"PHB",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$8B
{"STY",	LEN3,	OP_ABS},     //$8C
{"STA",	LEN3,	OP_ABS},     //$8D
{"STX",	LEN3,	OP_ABS},     //$8E
{"STA",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$8F
{"BCC",	LEN2,	OP_REL},    //$90
{"STA",	LEN2,	OP_ZP_IY},    //$91
{"STA",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$92
{"STA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$93
{"STY",	LEN2,	OP_ZP_X},    //$94
{"STA",	LEN2,	OP_ZP_X},    //$95
{"STX",	LEN2,	OP_ZP_Y},    //$96
{"STA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$97
{"TYA",	LEN1,	OP_NONE},     //$98
{"STA",	LEN3,	OP_ABS_Y},     //$99
{"TXS",	LEN1,	OP_NONE},     //$9A
{"TXY",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$9B
{"STZ",	(LEN3 | NOT_6502),	OP_ABS},     //$9C
{"STA",	LEN3,	OP_ABS_X},     //$9D
{"STZ",	(LEN3 | NOT_6502),	OP_ABS_X},     //$9E
{"STA",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_X_L},     //$9F
{"LDY",	(LEN2 | X_ADDS),	OP_IMM},     //$A0
{"LDA",	LEN2,	OP_ZP_XI},    //$A1
{"LDX",	(LEN2 | X_ADDS),	OP_IMM},     //$A2
{"LDA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$A3
{"LDY",	LEN2,	OP_ZP},    //$A4
{"LDA",	LEN2,	OP_ZP},    //$A5
{"LDX",	LEN2,	OP_ZP},    //$A6
{"LDA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$A7
{"TAY",	LEN1,	OP_NONE},     //$A8
{"LDA",	(LEN2 | M_ADDS),	OP_IMM},     //$A9
{"TAX",	LEN1,	OP_NONE},     //$AA
{"PLB",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$AB
{"LDY",	LEN3,	OP_ABS},     //$AC
{"LDA",	LEN3,	OP_ABS},     //$AD
{"LDX",	LEN3,	OP_ABS},     //$AE
{"LDA",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$AF
{"BCS",	LEN2,	OP_REL},    //$B0
{"LDA",	LEN2,	OP_ZP_IY},    //$B1
{"LDA",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$B2
{"LDA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$B3
{"LDY",	LEN2,	OP_ZP_X},    //$B4
{"LDA",	LEN2,	OP_ZP_X},    //$B5
{"LDX",	LEN2,	OP_ZP_Y},    //$B6
{"LDA",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$B7
{"CLV",	LEN1,	OP_NONE},     //$B8
{"LDA",	LEN3,	OP_ABS_Y},     //$B9
{"TSX",	LEN1,	OP_NONE},     //$BA
{"TYX",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$BB
{"LDY",	LEN3,	OP_ABS_X},     //$BC
{"LDA",	LEN3,	OP_ABS_X},     //$BD
{"LDX",	LEN3,	OP_ABS_Y},     //$BE
{"LDA",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_X_L},     //$BF
{"CPY",	(LEN2 | X_ADDS),	OP_IMM},     //$C0
{"CMP",	LEN2,	OP_ZP_XI},    //$C1
{"REP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_IMM},     //$C2
{"CMP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$C3
{"CPY",	LEN2,	OP_ZP},    //$C4
{"CMP",	LEN2,	OP_ZP},    //$C5
{"DEC",	LEN2,	OP_ZP},    //$C6
{"CMP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$C7
{"INY",	LEN1,	OP_NONE},     //$C8
{"CMP",	(LEN2 | M_ADDS),	OP_IMM},     //$C9
{"DEX",	LEN1,	OP_NONE},     //$CA
{"WAI",	(LEN1 | NOT_6502),	OP_NONE},     //$CB
{"CPY",	LEN3,	OP_ABS},     //$CC
{"CMP",	LEN3,	OP_ABS},     //$CD
{"DEC",	LEN3,	OP_ABS},     //$CE
{"CMP",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$CF
{"BNE",	LEN2,	OP_REL},    //$D0
{"CMP",	LEN2,	OP_ZP_IY},    //$D1
{"CMP",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$D2
{"CMP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$D3
{"PEI",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP},    //$D4
{"CMP",	LEN2,	OP_ZP_X},    //$D5
{"DEC",	LEN2,	OP_ZP_X},    //$D6
{"CMP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$D7
{"CLD",	LEN1,	OP_NONE},     //$D8
{"CMP",	LEN3,	OP_ABS_Y},     //$D9
{"PHX",	(LEN1 | NOT_6502),	OP_NONE},     //$DA
{"STP",	(LEN1 | NOT_6502),	OP_NONE},     //$DB
{"JML",	(LEN3 | NOT_65C02 | NOT_6502),	OP_ABS_IND_L},     //$DC
{"CMP",	LEN3,	OP_ABS_X},     //$DD
{"DEC",	LEN3,	OP_ABS_X},     //$DE
{"CMP",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_X_L},     //$DF
{"CPX",	(LEN2 | X_ADDS),	OP_IMM},     //$E0
{"SBC",	LEN2,	OP_ZP_XI},    //$E1
{"SEP",	(LEN2 | NOT_65C02 | NOT_6502),	OP_IMM},     //$E2
{"SBC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR},    //$E3
{"CPX",	LEN2,	OP_ZP},    //$E4
{"SBC",	LEN2,	OP_ZP},    //$E5
{"INC",	LEN2,	OP_ZP},    //$E6
{"SBC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IND_L},    //$E7
{"INX",	LEN1,	OP_NONE},     //$E8
{"SBC",	(LEN2 | M_ADDS),	OP_IMM},     //$E9
{"NOP",	LEN1,	OP_NONE},     //$EA
{"XBA",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$EB
{"INC",	LEN3,	OP_ABS},     //$EC
{"SBC",	LEN3,	OP_ABS},     //$ED
{"INC",	LEN3,	OP_ABS},     //$EE
{"SBC",	(LEN4 | NOT_65C02 | NOT_6502),	OP_ABS_L},     //$EF
{"BEQ",	LEN2,	OP_REL},    //$F0
{"SBC",	LEN2,	OP_ZP_IY},    //$F1
{"SBC",	(LEN2 | NOT_6502),	OP_ZP_IND},    //$F2
{"SBC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_SR_IY},    //$F3
{"PEA",	(LEN3 | NOT_65C02 | NOT_6502),	OP_IMM},     //$F4
{"SBC",	LEN2,	OP_ZP_X},    //$F5
{"INC",	LEN2,	OP_ZP_X},    //$F6
{"SBC",	(LEN2 | NOT_65C02 | NOT_6502),	OP_ZP_IY_L},    //$F7
{"SED",	LEN1,	OP_NONE},     //$F8
{"SBC",	LEN3,	OP_ABS_Y},     //$F9
{"PLX",	(LEN1 | NOT_6502),	OP_NONE},     //$FA
{"XCE",	(LEN1 | NOT_65C02 | NOT_6502),	OP_NONE},     //$FB
{"JSR",	(LEN3 | NOT_65C02 | NOT_6502),	OP_ABS_X_IND},    //$FC
{"SBC",	LEN3,	OP_ABS_X},     //$FD
{"INC",	LEN3,	OP_ABS_X},     //$FE
{"SBC",	LEN4,	OP_ABS_X_L}      //$FF
};



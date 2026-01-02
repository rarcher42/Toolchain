#include <stdio.h>
#include <stdint.h>
#include "optbl_65816.h"
#include "sim.h"

// sizeinfo field: includes modifiers for M and X flags
const uint8_t LEN1 = 0x01;
const uint8_t LEN2 = 0x02;
const uint8_t LEN3 = 0x03;
const uint8_t LEN4 = 0x04;
const uint8_t M_ADDS = 0x80;    // If M flag is 0, add 1 to ilen
const uint8_t X_ADDS = 0x40;    // If X = 0, then add 1 to inst len

const uint8_t NOT_65C02 = 0x01;     // Instruction not supported 65c02
const uint8_t NOT_6502 = 0x02;      // Instruction not supported NMOS
const uint8_t AL = 0x0;             // All supported
const uint8_t N2 = (NOT_6502);      // Only 6502 not supported
const uint8_t NB = (NOT_6502 | NOT_65C02);  // Only 65816 supported

op_tbl opcode_table[] = {
{"BRK", AL, LEN1, OP_NONE},     //$00
{"ORA", AL, LEN2, OP_ZP_XI},    //$01
{"COP", NB, LEN2, OP_IMM},     //$02
{"ORA", NB, LEN2, OP_SR},    //$03
{"TSB", N2, LEN2, OP_ZP},    //$04
{"ORA", AL, LEN2, OP_ZP},    //$05
{"ASL", AL, LEN2, OP_ZP},    //$06
{"ORA", NB, LEN2, OP_ZP_IND_L},    //$07
{"PHP", AL, LEN1, OP_STACK},     //$08
{"ORA", AL, LEN2|M_ADDS, OP_IMM},     //$09
{"ASL", AL, LEN1, OP_A},     //$0A
{"PHD", NB, LEN1, OP_STACK},     //$0B
{"TSB", N2, LEN3, OP_ABS},     //$0C
{"ORA", AL, LEN3, OP_ABS},     //$0D
{"ASL", AL, LEN3, OP_ABS},     //$0E
{"ORA", NB, LEN4, OP_ABS_L},     //$0F
{"BPL", AL, LEN2, OP_REL},    //$10
{"ORA", AL, LEN2, OP_ZP_IY},    //$11
{"ORA", N2, LEN2, OP_ZP_IND},    //$12
{"ORA", NB, LEN2, OP_SR_IY},    //$13
{"TRB", N2, LEN2, OP_ZP},    //$14
{"ORA", AL, LEN2, OP_ZP_X},    //$15
{"ASL", AL, LEN2, OP_ZP_X},    //$16
{"ORA", NB, LEN2, OP_ZP_IY_L},    //$17
{"CLC", AL, LEN1, OP_NONE},     //$18
{"ORA", AL, LEN3, OP_ABS_Y},     //$19
{"INC", N2, LEN1, OP_A},     //$1A
{"TCS", NB, LEN1, OP_NONE},     //$1B
{"TRB", N2, LEN3, OP_ABS},     //$1C
{"ORA", AL, LEN3, OP_ABS_X},     //$1D
{"ASL", AL, LEN3, OP_ABS_X},     //$1E
{"ORA", NB, LEN4, OP_ABS_X_L},     //$1F
{"JSR", AL, LEN3, OP_ABS},     //$20
{"AND", AL, LEN2, OP_ZP_XI},    //$21
{"JSL", NB, LEN4, OP_ABS_L},     //$22
{"AND", NB, LEN2, OP_SR},    //$23
{"BIT", AL, LEN2, OP_ZP},    //$24
{"AND", AL, LEN2, OP_ZP},    //$25
{"ROL", AL, LEN2, OP_ZP},    //$26
{"AND", NB, LEN2, OP_ZP_IND_L},    //$27
{"PLP", AL, LEN1, OP_STACK},     //$28
{"AND", AL, LEN2|M_ADDS, OP_IMM},     //$29
{"ROL", AL, LEN1, OP_A},     //$2A
{"PLD", NB, LEN1, OP_STACK},     //$2B
{"BIT", AL, LEN3, OP_ABS},     //$2C
{"AND", AL, LEN3, OP_ABS},     //$2D
{"ROL", AL, LEN3, OP_ABS},     //$2E
{"AND", NB, LEN4, OP_ABS_L},     //$2F
{"BMI", AL, LEN2, OP_REL},    //$30
{"AND", AL, LEN2, OP_ZP_IY},    //$31
{"AND", N2, LEN2, OP_ZP_IND},    //$32
{"AND", NB, LEN2, OP_SR_IY},    //$33
{"BIT", N2, LEN2, OP_ZP_X},    //$34
{"AND", AL, LEN2, OP_ZP_X},    //$35
{"ROL", AL, LEN2, OP_ZP_X},    //$36
{"AND", NB, LEN2, OP_ZP_IY_L},    //$37
{"SEC", AL, LEN1, OP_NONE},     //$38
{"AND", AL, LEN3, OP_ABS_Y},     //$39
{"DEC", N2, LEN1, OP_A},     //$3A
{"TSC", NB, LEN1, OP_NONE},     //$3B
{"BIT", N2, LEN3, OP_ABS_X},     //$3C
{"AND", AL, LEN3, OP_ABS_X},     //$3D
{"ROL", AL, LEN3, OP_ABS_X},     //$3E
{"AND", N2, LEN4, OP_ABS_X_L},     //$3F
{"RTI", AL, LEN1, OP_NONE},     //$40
{"EOR", NB, LEN2, OP_ZP_XI},    //$41
{"WDM", NB, LEN2, OP_IMM},     //$42
{"EOR", NB, LEN2, OP_SR},    //$43
{"MVP", NB, LEN3, OP_IMM},    //$44
{"EOR", AL, LEN2, OP_ZP},    //$45
{"LSR", AL, LEN2, OP_ZP},    //$46
{"EOR", NB, LEN2, OP_ZP_IND_L},    //$47
{"PHA", AL, LEN1, OP_STACK},     //$48
{"EOR", AL, LEN2|M_ADDS, OP_IMM},     //$49
{"LSR", AL, LEN1, OP_A},     //$4A
{"PHK", NB, LEN1, OP_STACK},     //$4B
{"JMP", AL, LEN3, OP_ABS},     //$4C
{"EOR", AL, LEN3, OP_ABS},     //$4D
{"LSR", AL, LEN3, OP_ABS},     //$4E
{"EOR", NB, LEN4, OP_ABS_L},     //$4F
{"BVC", AL, LEN2, OP_REL},    //$50
{"EOR", AL, LEN2, OP_ZP_IY},    //$51
{"EOR", N2, LEN2, OP_ZP_IND},    //$52
{"EOR", N2, LEN2, OP_SR_IY},    //$53
{"MVN", NB, LEN3, OP_IMM},    //$54
{"EOR", AL, LEN2, OP_ZP_X},    //$55
{"LSR", AL, LEN2, OP_ZP_X},    //$56
{"EOR", NB, LEN2, OP_ZP_IY_L},    //$57
{"CLI", AL, LEN1, OP_NONE},     //$58
{"EOR", AL, LEN3, OP_ABS_Y},     //$59
{"PHY", N2, LEN1, OP_STACK},     //$5A
{"TCD", NB, LEN1, OP_NONE},     //$5B
{"JML", NB, LEN4, OP_ABS_L},     //$5C
{"EOR", AL, LEN3, OP_ABS_X},     //$5D
{"LSR", AL, LEN3, OP_ABS_X},     //$5E
{"EOR", NB, LEN4, OP_ABS_X_L},     //$5F
{"RTS", AL, LEN1, OP_NONE},     //$60
{"ADC", AL, LEN2, OP_ZP_XI},    //$61
{"PER", NB, LEN3, OP_REL_L},    //$62
{"ADC", NB, LEN2, OP_SR},    //$63
{"STZ", N2, LEN2, OP_ZP},    //$64
{"ADC", AL, LEN2, OP_ZP},    //$65
{"ROR", AL, LEN2, OP_ZP},    //$66
{"ADC", NB, LEN2, OP_ZP_IND_L},    //$67
{"PLA", AL, LEN1, OP_STACK},     //$68
{"ADC", AL, LEN2|M_ADDS, OP_IMM},     //$69
{"ROR", AL, LEN1, OP_A},     //$6A
{"RTL", NB, LEN1, OP_NONE},     //$6B
{"JMP", AL, LEN3, OP_ABS_IND},     //$6C
{"ADC", AL, LEN3, OP_ABS},     //$6D
{"ROR", AL, LEN3, OP_ABS},     //$6E
{"ADC", NB, LEN4, OP_ABS_L},     //$6F
{"BVS", AL, LEN2, OP_REL},    //$70
{"ADC", AL, LEN2, OP_ZP_IY},    //$71
{"ADC", AL, LEN2, OP_ZP_IND},    //$72
{"ADC", NB, LEN2, OP_SR_IY},    //$73
{"STZ", N2, LEN2, OP_ZP_X},    //$74
{"ADC", AL, LEN2, OP_ZP_X},    //$75
{"ROR", AL, LEN2, OP_ZP_X},    //$76
{"ADC", NB, LEN2, OP_ZP_IY_L},    //$77
{"SEI", AL, LEN1, OP_NONE},     //$78
{"ADC", AL, LEN3, OP_ABS_Y},     //$79
{"PLY", N2, LEN1, OP_STACK},     //$7A
{"TDC", NB, LEN1, OP_NONE},     //$7B
{"JMP", N2, LEN3, OP_ABS_X_IND},    //$7C
{"ADC", AL, LEN3, OP_ABS_X},     //$7D
{"ROR", AL, LEN3, OP_ABS_X},     //$7E
{"ADC", NB, LEN4, OP_ABS_X_L},     //$7F
{"BRA", N2, LEN2, OP_REL},    //$80
{"STA", AL, LEN2, OP_ZP_XI},    //$81
{"BRL", NB, LEN3, OP_REL_L},    //$82
{"STA", NB, LEN2, OP_SR},    //$83
{"STY", AL, LEN2, OP_ZP},    //$84
{"STA", AL, LEN2, OP_ZP},    //$85
{"STX", AL, LEN2, OP_ZP},    //$86
{"STA", NB, LEN2, OP_ZP_IND_L},    //$87
{"DEY", AL, LEN1, OP_NONE},     //$88
{"BIT", AL, LEN2|M_ADDS, OP_IMM},     //$89
{"TXA", AL, LEN1, OP_NONE},     //$8A
{"PHB", NB, LEN1, OP_STACK},     //$8B
{"STY", AL, LEN3, OP_ABS},     //$8C
{"STA", AL, LEN3, OP_ABS},     //$8D
{"STX", AL, LEN3, OP_ABS},     //$8E
{"STA", NB, LEN4, OP_ABS_L},     //$8F
{"BCC", AL, LEN2, OP_REL},    //$90
{"STA", AL, LEN2, OP_ZP_IY},    //$91
{"STA", N2, LEN2, OP_ZP_IND},    //$92
{"STA", NB, LEN2, OP_SR_IY},    //$93
{"STY", AL, LEN2, OP_ZP_X},    //$94
{"STA", AL, LEN2, OP_ZP_X},    //$95
{"STX", AL, LEN2, OP_ZP_Y},    //$96
{"STA", NB, LEN2, OP_ZP_IY_L},    //$97
{"TYA", AL, LEN1, OP_NONE},     //$98
{"STA", AL, LEN3, OP_ABS_Y},     //$99
{"TXS", AL, LEN1, OP_NONE},     //$9A
{"TXY", NB, LEN1, OP_NONE},     //$9B
{"STZ", N2, LEN3, OP_ABS},     //$9C
{"STA", AL, LEN3, OP_ABS_X},     //$9D
{"STZ", N2, LEN3, OP_ABS_X},     //$9E
{"STA", NB, LEN4, OP_ABS_X_L},     //$9F
{"LDY", AL, LEN2|X_ADDS, OP_IMM},     //$A0
{"LDA", AL, LEN2, OP_ZP_XI},    //$A1
{"LDX", AL, LEN2|X_ADDS, OP_IMM},     //$A2
{"LDA", NB, LEN2, OP_SR},    //$A3
{"LDY", AL, LEN2, OP_ZP},    //$A4
{"LDA", AL, LEN2, OP_ZP},    //$A5
{"LDX", AL, LEN2, OP_ZP},    //$A6
{"LDA", NB, LEN2, OP_ZP_IND_L},    //$A7
{"TAY", AL, LEN1, OP_NONE},     //$A8
{"LDA", AL, LEN2|M_ADDS, OP_IMM},     //$A9
{"TAX", AL, LEN1, OP_NONE},     //$AA
{"PLB", NB, LEN1, OP_STACK},     //$AB
{"LDY", AL, LEN3, OP_ABS},     //$AC
{"LDA", AL, LEN3, OP_ABS},     //$AD
{"LDX", AL, LEN3, OP_ABS},     //$AE
{"LDA", NB, LEN4, OP_ABS_L},     //$AF
{"BCS", AL, LEN2, OP_REL},    //$B0
{"LDA", AL, LEN2, OP_ZP_IY},    //$B1
{"LDA", N2, LEN2, OP_ZP_IND},    //$B2
{"LDA", NB, LEN2, OP_SR_IY},    //$B3
{"LDY", AL, LEN2, OP_ZP_X},    //$B4
{"LDA", AL, LEN2, OP_ZP_X},    //$B5
{"LDX", AL, LEN2, OP_ZP_Y},    //$B6
{"LDA", NB, LEN2, OP_ZP_IY_L},    //$B7
{"CLV", AL, LEN1, OP_NONE},     //$B8
{"LDA", AL, LEN3, OP_ABS_Y},     //$B9
{"TSX", AL, LEN1, OP_NONE},     //$BA
{"TYX", NB, LEN1, OP_NONE},     //$BB
{"LDY", AL, LEN3, OP_ABS_X},     //$BC
{"LDA", AL, LEN3, OP_ABS_X},     //$BD
{"LDX", AL, LEN3, OP_ABS_Y},     //$BE
{"LDA", NB, LEN4, OP_ABS_X_L},     //$BF
{"CPY", AL, LEN2|X_ADDS, OP_IMM},     //$C0
{"CMP", AL, LEN2, OP_ZP_XI},    //$C1
{"REP", NB, LEN2, OP_IMM},     //$C2
{"CMP", NB, LEN2, OP_SR},    //$C3
{"CPY", AL, LEN2, OP_ZP},    //$C4
{"CMP", AL, LEN2, OP_ZP},    //$C5
{"DEC", AL, LEN2, OP_ZP},    //$C6
{"CMP", NB, LEN2, OP_ZP_IND_L},    //$C7
{"INY", AL, LEN1, OP_NONE},     //$C8
{"CMP", AL, LEN2|M_ADDS, OP_IMM},     //$C9
{"DEX", AL, LEN1, OP_NONE},     //$CA
{"WAI", N2, LEN1, OP_NONE},     //$CB
{"CPY", AL, LEN3, OP_ABS},     //$CC
{"CMP", AL, LEN3, OP_ABS},     //$CD
{"DEC", AL, LEN3, OP_ABS},     //$CE
{"CMP", NB, LEN4, OP_ABS_L},     //$CF
{"BNE", AL, LEN2, OP_REL},    //$D0
{"CMP", AL, LEN2, OP_ZP_IY},    //$D1
{"CMP", N2, LEN2, OP_ZP_IND},    //$D2
{"CMP", NB, LEN2, OP_SR_IY},    //$D3
{"PEI", NB, LEN2, OP_ZP},    //$D4
{"CMP", AL, LEN2, OP_ZP_X},    //$D5
{"DEC", AL, LEN2, OP_ZP_X},    //$D6
{"CMP", NB, LEN2, OP_ZP_IY_L},    //$D7
{"CLD", AL, LEN1, OP_NONE},     //$D8
{"CMP", AL, LEN3, OP_ABS_Y},     //$D9
{"PHX", N2, LEN1, OP_STACK},     //$DA
{"STP", N2, LEN1, OP_NONE},     //$DB
{"JML", NB, LEN3, OP_ABS_IND_L},     //$DC
{"CMP", AL, LEN3, OP_ABS_X},     //$DD
{"DEC", AL, LEN3, OP_ABS_X},     //$DE
{"CMP", NB, LEN4, OP_ABS_X_L},     //$DF
{"CPX", AL, LEN2|X_ADDS, OP_IMM},     //$E0
{"SBC", AL, LEN2, OP_ZP_XI},    //$E1
{"SEP", NB, LEN2, OP_IMM},     //$E2
{"SBC", NB, LEN2, OP_SR},    //$E3
{"CPX", AL, LEN2,  OP_ZP},    //$E4
{"SBC", AL, LEN2,  OP_ZP},    //$E5
{"INC", AL, LEN2,  OP_ZP},    //$E6
{"SBC", NB, LEN2, OP_ZP_IND_L},    //$E7
{"INX", AL, LEN1, OP_NONE},     //$E8
{"SBC", AL, LEN2|M_ADDS, OP_IMM},     //$E9
{"NOP", AL, LEN1, OP_NONE},     //$EA
{"XBA", NB, LEN1, OP_NONE},     //$EB
{"INC", AL, LEN3, OP_ABS},     //$EC
{"SBC", AL, LEN3, OP_ABS},     //$ED
{"INC", AL, LEN3, OP_ABS},     //$EE
{"SBC", NB, LEN4, OP_ABS_L},     //$EF
{"BEQ", AL, LEN2, OP_REL},    //$F0
{"SBC", AL, LEN2, OP_ZP_IY},    //$F1
{"SBC", N2, LEN2, OP_ZP_IND},    //$F2
{"SBC", NB, LEN2, OP_SR_IY},    //$F3
{"PEA", NB, LEN3, OP_IMM},     //$F4
{"SBC", AL, LEN2, OP_ZP_X},    //$F5
{"INC", AL, LEN2, OP_ZP_X},    //$F6
{"SBC", NB, LEN2, OP_ZP_IY_L},    //$F7
{"SED", AL, LEN1, OP_NONE},     //$F8
{"SBC", AL, LEN3, OP_ABS_Y},     //$F9
{"PLX", N2, LEN1, OP_STACK},     //$FA
{"XCE", NB, LEN1, OP_NONE},     //$FB
{"JSR", NB, LEN3, OP_ABS_X_IND},    //$FC
{"SBC", AL, LEN3, OP_ABS_X},     //$FD
{"INC", AL, LEN3, OP_ABS_X},     //$FE
{"SBC", NB, LEN4, OP_ABS_X_L}      //$FF
};


char *get_mnemonic (uint8_t op)
{
    return opcode_table[op].ops;
}

address_mode_t get_addr_mode (uint8_t op)
{
    return opcode_table[op].adm;
}

uint8_t get_oplen (uint8_t op) 
{
    uint8_t sizeinfo;
    uint8_t oplen;
    uint8_t unsupport;
    
    sizeinfo = opcode_table[op].sizeinfo;
    unsupport = opcode_table[op].unsupport;
 
    if ((get_cpu_type() == 1) && (unsupport & NOT_6502)) { 
        printf("Unimplemented NMOS 6502 opcode $%02X\n", op);
    return 0;    // Not implemented!
    }
    if ((get_cpu_type() == 2) && (unsupport & NOT_65C02)) {
        printf("Unimplemented CMOS 65c02 opcode $%02X\n", op);
    return 0;    // Not implemented!
    }
    oplen = sizeinfo & 0x7;  // Extract length bits
   
    if (sizeinfo & M_ADDS) {
    // Instruction:  add 1 byte if M flag is set
        if (GET_FLAG(M_FLAG) == 0) {
            ++oplen;
        }
    }
    if (sizeinfo & X_ADDS)  {
        if (GET_FLAG(X_FLAG) == 0) {
            ++oplen;
        }
    }
    return oplen;
}



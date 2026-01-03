#include <stdio.h>
#include <stdint.h>
#include "optbl_65816.h"
#include "sim.h"
#include "opcodes.h"

// sizeinfo field: includes modifiers for M and X flags
const uint8_t LEN1 = 0x01;
const uint8_t LEN2 = 0x02;
const uint8_t LEN3 = 0x03;
const uint8_t LEN4 = 0x04;
const uint8_t M_A = 0x80;    // If M flag is 0, add 1 to ilen
const uint8_t X_A = 0x40;    // If X = 0, then add 1 to inst len

const uint8_t NOT_65C02 = 0x01;     // Instruction not supported 65c02
const uint8_t NOT_6502 = 0x02;      // Instruction not supported NMOS
const uint8_t AL = 0x0;             // All supported
const uint8_t N2 = (NOT_6502);      // Only 6502 not supported
const uint8_t NB = (NOT_6502 | NOT_65C02);  // Only 65816 supported

op_tbl opcode_table[] = {
{"BRK", AL, LEN1,       OP_NONE,        brk},       //$00
{"ORA", AL, LEN2,       OP_ZP_XI,       unimp},     //$01
{"COP", NB, LEN2,       OP_IMM,         unimp},     //$02
{"ORA", NB, LEN2,       OP_SR,          unimp},     //$03
{"TSB", N2, LEN2,       OP_ZP,          unimp},     //$04
{"ORA", AL, LEN2,       OP_ZP,          unimp},     //$05
{"ASL", AL, LEN2,       OP_ZP,          unimp},     //$06
{"ORA", NB, LEN2,       OP_ZP_IND_L,    unimp},     //$07
{"PHP", AL, LEN1,       OP_STK,         unimp},     //$08
{"ORA", AL, LEN2|M_A,   OP_IMM,         unimp},     //$09
{"ASL", AL, LEN1,       OP_A,           unimp},     //$0A
{"PHD", NB, LEN1,       OP_STK,         unimp},     //$0B
{"TSB", N2, LEN3,       OP_ABS,         unimp},     //$0C
{"ORA", AL, LEN3,       OP_ABS,         unimp},     //$0D
{"ASL", AL, LEN3,       OP_ABS,         unimp},     //$0E
{"ORA", NB, LEN4,       OP_ABS_L,       unimp},     //$0F
{"BPL", AL, LEN2,       OP_REL,         unimp},     //$10
{"ORA", AL, LEN2,       OP_ZP_IY,       unimp},     //$11
{"ORA", N2, LEN2,       OP_ZP_IND,      unimp},     //$12
{"ORA", NB, LEN2,       OP_SR_IY,       unimp},     //$13
{"TRB", N2, LEN2,       OP_ZP,          unimp},     //$14
{"ORA", AL, LEN2,       OP_ZP_X,        unimp},     //$15
{"ASL", AL, LEN2,       OP_ZP_X,        unimp},     //$16
{"ORA", NB, LEN2,       OP_ZP_IY_L,     unimp},     //$17
{"CLC", AL, LEN1,       OP_NONE,        clc},       //$18
{"ORA", AL, LEN3,       OP_ABS_Y,       unimp},     //$19
{"INC", N2, LEN1,       OP_A,           unimp},     //$1A
{"TCS", NB, LEN1,       OP_NONE,        unimp},     //$1B
{"TRB", N2, LEN3,       OP_ABS,         unimp},     //$1C
{"ORA", AL, LEN3,       OP_ABS_X,       unimp},     //$1D
{"ASL", AL, LEN3,       OP_ABS_X,       unimp},     //$1E
{"ORA", NB, LEN4,       OP_ABS_X_L,     unimp},     //$1F
{"JSR", AL, LEN3,       OP_ABS,         unimp},     //$20
{"AND", AL, LEN2,       OP_ZP_XI,       unimp},     //$21
{"JSL", NB, LEN4,       OP_ABS_L,       unimp},     //$22
{"AND", NB, LEN2,       OP_SR,          unimp},     //$23
{"BIT", AL, LEN2,       OP_ZP,          unimp},     //$24
{"AND", AL, LEN2,       OP_ZP,          unimp},     //$25
{"ROL", AL, LEN2,       OP_ZP,          unimp},     //$26
{"AND", NB, LEN2,       OP_ZP_IND_L,    unimp},     //$27
{"PLP", AL, LEN1,       OP_STK,         unimp},     //$28
{"AND", AL, LEN2|M_A,   OP_IMM,         unimp},     //$29
{"ROL", AL, LEN1,       OP_A,           unimp},     //$2A
{"PLD", NB, LEN1,       OP_STK,         unimp},     //$2B
{"BIT", AL, LEN3,       OP_ABS,         unimp},     //$2C
{"AND", AL, LEN3,       OP_ABS,         unimp},     //$2D
{"ROL", AL, LEN3,       OP_ABS,         unimp},     //$2E
{"AND", NB, LEN4,       OP_ABS_L,       unimp},     //$2F
{"BMI", AL, LEN2,       OP_REL,         unimp},     //$30
{"AND", AL, LEN2,       OP_ZP_IY,       unimp},     //$31
{"AND", N2, LEN2,       OP_ZP_IND,      unimp},     //$32
{"AND", NB, LEN2,       OP_SR_IY,       unimp},     //$33
{"BIT", N2, LEN2,       OP_ZP_X,        unimp},     //$34
{"AND", AL, LEN2,       OP_ZP_X,        unimp},     //$35
{"ROL", AL, LEN2,       OP_ZP_X,        unimp},     //$36
{"AND", NB, LEN2,       OP_ZP_IY_L,     unimp},     //$37
{"SEC", AL, LEN1,       OP_NONE,        sec},       //$38
{"AND", AL, LEN3,       OP_ABS_Y,       unimp},     //$39
{"DEC", N2, LEN1,       OP_A,           unimp},     //$3A
{"TSC", NB, LEN1,       OP_NONE,        unimp},     //$3B
{"BIT", N2, LEN3,       OP_ABS_X,       unimp},     //$3C
{"AND", AL, LEN3,       OP_ABS_X,       unimp},     //$3D
{"ROL", AL, LEN3,       OP_ABS_X,       unimp},     //$3E
{"AND", N2, LEN4,       OP_ABS_X_L,     unimp},     //$3F
{"RTI", AL, LEN1,       OP_NONE,        unimp},     //$40
{"EOR", NB, LEN2,       OP_ZP_XI,       unimp},     //$41
{"WDM", NB, LEN2,       OP_IMM,         unimp},     //$42
{"EOR", NB, LEN2,       OP_SR,          unimp},     //$43
{"MVP", NB, LEN3,       OP_IMM,         unimp},     //$44
{"EOR", AL, LEN2,       OP_ZP,          unimp},     //$45
{"LSR", AL, LEN2,       OP_ZP,          unimp},     //$46
{"EOR", NB, LEN2,       OP_ZP_IND_L,    unimp},     //$47
{"PHA", AL, LEN1,       OP_STK,         unimp},     //$48
{"EOR", AL, LEN2|M_A,   OP_IMM,         unimp},     //$49
{"LSR", AL, LEN1,       OP_A,           unimp},     //$4A
{"PHK", NB, LEN1,       OP_STK,         unimp},     //$4B
{"JMP", AL, LEN3,       OP_ABS,         unimp},     //$4C
{"EOR", AL, LEN3,       OP_ABS,         unimp},     //$4D
{"LSR", AL, LEN3,       OP_ABS,         unimp},     //$4E
{"EOR", NB, LEN4,       OP_ABS_L,       unimp},     //$4F
{"BVC", AL, LEN2,       OP_REL,         unimp},     //$50
{"EOR", AL, LEN2,       OP_ZP_IY,       unimp},     //$51
{"EOR", N2, LEN2,       OP_ZP_IND,      unimp},     //$52
{"EOR", N2, LEN2,       OP_SR_IY,       unimp},     //$53
{"MVN", NB, LEN3,       OP_IMM,         unimp},     //$54
{"EOR", AL, LEN2,       OP_ZP_X,        unimp},     //$55
{"LSR", AL, LEN2,       OP_ZP_X,        unimp},     //$56
{"EOR", NB, LEN2,       OP_ZP_IY_L,     unimp},     //$57
{"CLI", AL, LEN1,       OP_NONE,        cli},       //$58
{"EOR", AL, LEN3,       OP_ABS_Y,       unimp},     //$59
{"PHY", N2, LEN1,       OP_STK,         unimp},     //$5A
{"TCD", NB, LEN1,       OP_NONE,        unimp},     //$5B
{"JML", NB, LEN4,       OP_ABS_L,       unimp},     //$5C
{"EOR", AL, LEN3,       OP_ABS_X,       unimp},     //$5D
{"LSR", AL, LEN3,       OP_ABS_X,       unimp},     //$5E
{"EOR", NB, LEN4,       OP_ABS_X_L,     unimp},     //$5F
{"RTS", AL, LEN1,       OP_NONE,        unimp},     //$60
{"ADC", AL, LEN2,       OP_ZP_XI,       unimp},     //$61
{"PER", NB, LEN3,       OP_REL_L,       unimp},     //$62
{"ADC", NB, LEN2,       OP_SR,          unimp},     //$63
{"STZ", N2, LEN2,       OP_ZP,          unimp},     //$64
{"ADC", AL, LEN2,       OP_ZP,          unimp},     //$65
{"ROR", AL, LEN2,       OP_ZP,          unimp},     //$66
{"ADC", NB, LEN2,       OP_ZP_IND_L,    unimp},     //$67
{"PLA", AL, LEN1,       OP_STK,         unimp},     //$68
{"ADC", AL, LEN2|M_A,   OP_IMM,         unimp},     //$69
{"ROR", AL, LEN1,       OP_A,           unimp},     //$6A
{"RTL", NB, LEN1,       OP_NONE,        unimp},     //$6B
{"JMP", AL, LEN3,       OP_ABS_IND,     unimp},     //$6C
{"ADC", AL, LEN3,       OP_ABS,         unimp},     //$6D
{"ROR", AL, LEN3,       OP_ABS,         unimp},     //$6E
{"ADC", NB, LEN4,       OP_ABS_L,       unimp},     //$6F
{"BVS", AL, LEN2,       OP_REL,         unimp},     //$70
{"ADC", AL, LEN2,       OP_ZP_IY,       unimp},     //$71
{"ADC", AL, LEN2,       OP_ZP_IND,      unimp},     //$72
{"ADC", NB, LEN2,       OP_SR_IY,       unimp},     //$73
{"STZ", N2, LEN2,       OP_ZP_X,        unimp},     //$74
{"ADC", AL, LEN2,       OP_ZP_X,        unimp},     //$75
{"ROR", AL, LEN2,       OP_ZP_X,        unimp},     //$76
{"ADC", NB, LEN2,       OP_ZP_IY_L,     unimp},     //$77
{"SEI", AL, LEN1,       OP_NONE,        sei},       //$78
{"ADC", AL, LEN3,       OP_ABS_Y,       unimp},     //$79
{"PLY", N2, LEN1,       OP_STK,         unimp},     //$7A
{"TDC", NB, LEN1,       OP_NONE,        unimp},     //$7B
{"JMP", N2, LEN3,       OP_ABS_X_IND,   unimp},     //$7C
{"ADC", AL, LEN3,       OP_ABS_X,       unimp},     //$7D
{"ROR", AL, LEN3,       OP_ABS_X,       unimp},     //$7E
{"ADC", NB, LEN4,       OP_ABS_X_L,     unimp},     //$7F
{"BRA", N2, LEN2,       OP_REL,         unimp},     //$80
{"STA", AL, LEN2,       OP_ZP_XI,       sta},       //$81
{"BRL", NB, LEN3,       OP_REL_L,       unimp},     //$82
{"STA", NB, LEN2,       OP_SR,          sta},       //$83
{"STY", AL, LEN2,       OP_ZP,          sty},       //$84
{"STA", AL, LEN2,       OP_ZP,          sta},       //$85
{"STX", AL, LEN2,       OP_ZP,          stx},       //$86
{"STA", NB, LEN2,       OP_ZP_IND_L,    sta},       //$87
{"DEY", AL, LEN1,       OP_NONE,        unimp},     //$88
{"BIT", AL, LEN2|M_A,   OP_IMM,         unimp},     //$89
{"TXA", AL, LEN1,       OP_NONE,        unimp},     //$8A
{"PHB", NB, LEN1,       OP_STK,         unimp},     //$8B
{"STY", AL, LEN3,       OP_ABS,         sty},       //$8C
{"STA", AL, LEN3,       OP_ABS,         sta},       //$8D
{"STX", AL, LEN3,       OP_ABS,         stx},       //$8E
{"STA", NB, LEN4,       OP_ABS_L,       sta},       //$8F
{"BCC", AL, LEN2,       OP_REL,         unimp},     //$90
{"STA", AL, LEN2,       OP_ZP_IY,       sta},       //$91
{"STA", N2, LEN2,       OP_ZP_IND,      sta},       //$92
{"STA", NB, LEN2,       OP_SR_IY,       sta},       //$93
{"STY", AL, LEN2,       OP_ZP_X,        sty},       //$94
{"STA", AL, LEN2,       OP_ZP_X,        sta},       //$95
{"STX", AL, LEN2,       OP_ZP_Y,        stx},       //$96
{"STA", NB, LEN2,       OP_ZP_IY_L,     sta},       //$97
{"TYA", AL, LEN1,       OP_NONE,        unimp},     //$98
{"STA", AL, LEN3,       OP_ABS_Y,       sta},       //$99
{"TXS", AL, LEN1,       OP_NONE,        unimp},     //$9A
{"TXY", NB, LEN1,       OP_NONE,        unimp},     //$9B
{"STZ", N2, LEN3,       OP_ABS,         unimp},     //$9C
{"STA", AL, LEN3,       OP_ABS_X,       sta},       //$9D
{"STZ", N2, LEN3,       OP_ABS_X,       unimp},     //$9E
{"STA", NB, LEN4,       OP_ABS_X_L,     sta},       //$9F
{"LDY", AL, LEN2|X_A,   OP_IMM,         ldy},       //$A0
{"LDA", AL, LEN2,       OP_ZP_XI,       lda},       //$A1
{"LDX", AL, LEN2|X_A,   OP_IMM,         ldx},       //$A2
{"LDA", NB, LEN2,       OP_SR,          lda},       //$A3
{"LDY", AL, LEN2,       OP_ZP,          ldy},       //$A4
{"LDA", AL, LEN2,       OP_ZP,          lda},       //$A5
{"LDX", AL, LEN2,       OP_ZP,          ldx},       //$A6
{"LDA", NB, LEN2,       OP_ZP_IND_L,    lda},       //$A7
{"TAY", AL, LEN1,       OP_NONE,        unimp},     //$A8
{"LDA", AL, LEN2|M_A,   OP_IMM,         lda},       //$A9
{"TAX", AL, LEN1,       OP_NONE,        unimp},     //$AA
{"PLB", NB, LEN1,       OP_STK,         unimp},     //$AB
{"LDY", AL, LEN3,       OP_ABS,         ldy},       //$AC
{"LDA", AL, LEN3,       OP_ABS,         lda},       //$AD
{"LDX", AL, LEN3,       OP_ABS,         ldx},       //$AE
{"LDA", NB, LEN4,       OP_ABS_L,       lda},       //$AF
{"BCS", AL, LEN2,       OP_REL,         unimp},     //$B0
{"LDA", AL, LEN2,       OP_ZP_IY,       lda},       //$B1
{"LDA", N2, LEN2,       OP_ZP_IND,      lda},       //$B2
{"LDA", NB, LEN2,       OP_SR_IY,       lda},       //$B3
{"LDY", AL, LEN2,       OP_ZP_X,        ldy},       //$B4
{"LDA", AL, LEN2,       OP_ZP_X,        lda},       //$B5
{"LDX", AL, LEN2,       OP_ZP_Y,        ldx},       //$B6
{"LDA", NB, LEN2,       OP_ZP_IY_L,     lda},       //$B7
{"CLV", AL, LEN1,       OP_NONE,        clv},       //$B8
{"LDA", AL, LEN3,       OP_ABS_Y,       lda},       //$B9
{"TSX", AL, LEN1,       OP_NONE,        unimp},     //$BA
{"TYX", NB, LEN1,       OP_NONE,        unimp},     //$BB
{"LDY", AL, LEN3,       OP_ABS_X,       ldy},       //$BC
{"LDA", AL, LEN3,       OP_ABS_X,       lda},       //$BD
{"LDX", AL, LEN3,       OP_ABS_Y,       ldx},       //$BE
{"LDA", NB, LEN4,       OP_ABS_X_L,     lda},       //$BF
{"CPY", AL, LEN2|X_A,   OP_IMM,         unimp},     //$C0
{"CMP", AL, LEN2,       OP_ZP_XI,       unimp},     //$C1
{"REP", NB, LEN2,       OP_IMM,         rep},       //$C2
{"CMP", NB, LEN2,       OP_SR,          unimp},     //$C3
{"CPY", AL, LEN2,       OP_ZP,          unimp},     //$C4
{"CMP", AL, LEN2,       OP_ZP,          unimp},     //$C5
{"DEC", AL, LEN2,       OP_ZP,          unimp},     //$C6
{"CMP", NB, LEN2,       OP_ZP_IND_L,    unimp},     //$C7
{"INY", AL, LEN1,       OP_NONE,        unimp},     //$C8
{"CMP", AL, LEN2|M_A,   OP_IMM,         unimp},     //$C9
{"DEX", AL, LEN1,       OP_NONE,        unimp},     //$CA
{"WAI", N2, LEN1,       OP_NONE,        unimp},     //$CB
{"CPY", AL, LEN3,       OP_ABS,         unimp},     //$CC
{"CMP", AL, LEN3,       OP_ABS,         unimp},     //$CD
{"DEC", AL, LEN3,       OP_ABS,         unimp},     //$CE
{"CMP", NB, LEN4,       OP_ABS_L,       unimp},     //$CF
{"BNE", AL, LEN2,       OP_REL,         unimp},     //$D0
{"CMP", AL, LEN2,       OP_ZP_IY,       unimp},     //$D1
{"CMP", N2, LEN2,       OP_ZP_IND,      unimp},     //$D2
{"CMP", NB, LEN2,       OP_SR_IY,       unimp},     //$D3
{"PEI", NB, LEN2,       OP_ZP,          unimp},     //$D4
{"CMP", AL, LEN2,       OP_ZP_X,         unimp},    //$D5
{"DEC", AL, LEN2,       OP_ZP_X,        unimp},     //$D6
{"CMP", NB, LEN2,       OP_ZP_IY_L,     unimp},     //$D7
{"CLD", AL, LEN1,       OP_NONE,        cld},       //$D8
{"CMP", AL, LEN3,       OP_ABS_Y,       unimp},     //$D9
{"PHX", N2, LEN1,       OP_STK,         unimp},     //$DA
{"STP", N2, LEN1,       OP_NONE,        stp},       //$DB
{"JML", NB, LEN3,       OP_ABS_IND_L,   unimp},     //$DC
{"CMP", AL, LEN3,       OP_ABS_X,       unimp},     //$DD
{"DEC", AL, LEN3,       OP_ABS_X,       unimp},     //$DE
{"CMP", NB, LEN4,       OP_ABS_X_L,     unimp},     //$DF
{"CPX", AL, LEN2|X_A,   OP_IMM,         unimp},     //$E0
{"SBC", AL, LEN2,       OP_ZP_XI,       unimp},     //$E1
{"SEP", NB, LEN2,       OP_IMM,         sep},       //$E2
{"SBC", NB, LEN2,       OP_SR,          unimp},     //$E3
{"CPX", AL, LEN2,       OP_ZP,          unimp},     //$E4
{"SBC", AL, LEN2,       OP_ZP,          unimp},     //$E5
{"INC", AL, LEN2,       OP_ZP,          unimp},     //$E6
{"SBC", NB, LEN2,       OP_ZP_IND_L,    unimp},     //$E7
{"INX", AL, LEN1,       OP_NONE,        unimp},     //$E8
{"SBC", AL, LEN2|M_A,   OP_IMM,         unimp},     //$E9
{"NOP", AL, LEN1,       OP_NONE,        nop},       //$EA
{"XBA", NB, LEN1,       OP_NONE,        unimp},     //$EB
{"INC", AL, LEN3,       OP_ABS,         unimp},     //$EC
{"SBC", AL, LEN3,       OP_ABS,         unimp},     //$ED
{"INC", AL, LEN3,       OP_ABS,         unimp},     //$EE
{"SBC", NB, LEN4,       OP_ABS_L,       unimp},     //$EF
{"BEQ", AL, LEN2,       OP_REL,         unimp},     //$F0
{"SBC", AL, LEN2,       OP_ZP_IY,       unimp},     //$F1
{"SBC", N2, LEN2,       OP_ZP_IND,      unimp},     //$F2
{"SBC", NB, LEN2,       OP_SR_IY,       unimp},     //$F3
{"PEA", NB, LEN3,       OP_IMM,         unimp},     //$F4
{"SBC", AL, LEN2,       OP_ZP_X,        unimp},     //$F5
{"INC", AL, LEN2,       OP_ZP_X,        unimp},     //$F6
{"SBC", NB, LEN2,       OP_ZP_IY_L,     unimp},     //$F7
{"SED", AL, LEN1,       OP_NONE,        sed},       //$F8
{"SBC", AL, LEN3,       OP_ABS_Y,       unimp},     //$F9
{"PLX", N2, LEN1,       OP_STK,         unimp},     //$FA
{"XCE", NB, LEN1,       OP_NONE,        xce},       //$FB
{"JSR", NB, LEN3,       OP_ABS_X_IND,   unimp},     //$FC
{"SBC", AL, LEN3,       OP_ABS_X,       unimp},     //$FD
{"INC", AL, LEN3,       OP_ABS_X,       unimp},     //$FE
{"SBC", NB, LEN4,       OP_ABS_X_L,     unimp}      //$FF
};


char *get_mnemonic (uint8_t op)
{
    return opcode_table[op].ops;
}

void *get_op_function(uint8_t op)
{
    return opcode_table[op].do_op;
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
   
    if (sizeinfo & M_A) {
    // Instruction:  add 1 byte if M flag is set
        if (GET_FLAG(M_FLAG) == 0) {
            ++oplen;
        }
    }
    if (sizeinfo & X_A)  {
        if (GET_FLAG(X_FLAG) == 0) {
            ++oplen;
        }
    }
    return oplen;
}


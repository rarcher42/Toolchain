#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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
{"ORA", AL, LEN2,       OP_ZP_XI,       ora},       //$01
{"COP", NB, LEN2,       OP_IMM,         unimp},     //$02
{"ORA", NB, LEN2,       OP_SR,          ora},       //$03
{"TSB", N2, LEN2,       OP_ZP,          unimp},     //$04
{"ORA", AL, LEN2,       OP_ZP,          ora},       //$05
{"ASL", AL, LEN2,       OP_ZP,          asl},       //$06
{"ORA", NB, LEN2,       OP_ZP_IND_L,    ora},       //$07
{"PHP", AL, LEN1,       OP_STK,         php},       //$08
{"ORA", AL, LEN2|M_A,   OP_IMM,         ora},       //$09
{"ASL", AL, LEN1,       OP_A,           asl},       //$0A
{"PHD", NB, LEN1,       OP_STK,         phd},       //$0B
{"TSB", N2, LEN3,       OP_ABS,         unimp},     //$0C
{"ORA", AL, LEN3,       OP_ABS,         ora},       //$0D
{"ASL", AL, LEN3,       OP_ABS,         asl},       //$0E
{"ORA", NB, LEN4,       OP_ABS_L,       ora},       //$0F
{"BPL", AL, LEN2,       OP_REL,         bpl},       //$10
{"ORA", AL, LEN2,       OP_ZP_IY,       ora},       //$11
{"ORA", N2, LEN2,       OP_ZP_IND,      ora},       //$12
{"ORA", NB, LEN2,       OP_SR_IY,       ora},       //$13
{"TRB", N2, LEN2,       OP_ZP,          unimp},     //$14
{"ORA", AL, LEN2,       OP_ZP_X,        ora},       //$15
{"ASL", AL, LEN2,       OP_ZP_X,        asl},       //$16
{"ORA", NB, LEN2,       OP_ZP_IY_L,     ora},       //$17
{"CLC", AL, LEN1,       OP_NONE,        clc},       //$18
{"ORA", AL, LEN3,       OP_ABS_Y,       ora},       //$19
{"INC", N2, LEN1,       OP_A,           inc},       //$1A
{"TCS", NB, LEN1,       OP_NONE,        tcs},       //$1B
{"TRB", N2, LEN3,       OP_ABS,         unimp},     //$1C
{"ORA", AL, LEN3,       OP_ABS_X,       ora},       //$1D
{"ASL", AL, LEN3,       OP_ABS_X,       asl},       //$1E
{"ORA", NB, LEN4,       OP_ABS_X_L,     ora},       //$1F
{"JSR", AL, LEN3,       OP_ABS,         jsr},       //$20
{"AND", AL, LEN2,       OP_ZP_XI,       anl},       //$21
{"JSL", NB, LEN4,       OP_ABS_L,       jsl},       //$22
{"AND", NB, LEN2,       OP_SR,          anl},       //$23
{"BIT", AL, LEN2,       OP_ZP,          bit},       //$24
{"AND", AL, LEN2,       OP_ZP,          anl},       //$25
{"ROL", AL, LEN2,       OP_ZP,          rol},       //$26
{"AND", NB, LEN2,       OP_ZP_IND_L,    anl},       //$27
{"PLP", AL, LEN1,       OP_STK,         plp},       //$28
{"AND", AL, LEN2|M_A,   OP_IMM,         anl},       //$29
{"ROL", AL, LEN1,       OP_A,           rol},       //$2A
{"PLD", NB, LEN1,       OP_STK,         pld},       //$2B
{"BIT", AL, LEN3,       OP_ABS,         bit},       //$2C
{"AND", AL, LEN3,       OP_ABS,         anl},       //$2D
{"ROL", AL, LEN3,       OP_ABS,         rol},       //$2E
{"AND", NB, LEN4,       OP_ABS_L,       anl},       //$2F
{"BMI", AL, LEN2,       OP_REL,         bmi},       //$30
{"AND", AL, LEN2,       OP_ZP_IY,       anl},       //$31
{"AND", N2, LEN2,       OP_ZP_IND,      anl},       //$32
{"AND", NB, LEN2,       OP_SR_IY,       anl},       //$33
{"BIT", N2, LEN2,       OP_ZP_X,        bit},       //$34
{"AND", AL, LEN2,       OP_ZP_X,        anl},       //$35
{"ROL", AL, LEN2,       OP_ZP_X,        rol},       //$36
{"AND", NB, LEN2,       OP_ZP_IY_L,     anl},       //$37
{"SEC", AL, LEN1,       OP_NONE,        sec},       //$38
{"AND", AL, LEN3,       OP_ABS_Y,       anl},       //$39
{"DEC", N2, LEN1,       OP_A,           dec},       //$3A
{"TSC", NB, LEN1,       OP_NONE,        tsc},       //$3B
{"BIT", N2, LEN3,       OP_ABS_X,       bit},       //$3C
{"AND", AL, LEN3,       OP_ABS_X,       anl},       //$3D
{"ROL", AL, LEN3,       OP_ABS_X,       rol},       //$3E
{"AND", N2, LEN4,       OP_ABS_X_L,     anl},       //$3F
{"RTI", AL, LEN1,       OP_NONE,        rti},       //$40
{"EOR", NB, LEN2,       OP_ZP_XI,       eor},       //$41
{"WDM", NB, LEN2,       OP_IMM,         unimp},     //$42
{"EOR", NB, LEN2,       OP_SR,          eor},       //$43
{"MVP", NB, LEN3,       OP_IMM,         unimp},     //$44
{"EOR", AL, LEN2,       OP_ZP,          eor},       //$45
{"LSR", AL, LEN2,       OP_ZP,          lsr},       //$46
{"EOR", NB, LEN2,       OP_ZP_IND_L,    eor},       //$47
{"PHA", AL, LEN1,       OP_STK,         pha},       //$48
{"EOR", AL, LEN2|M_A,   OP_IMM,         eor},       //$49
{"LSR", AL, LEN1,       OP_A,           lsr},       //$4A
{"PHK", NB, LEN1,       OP_STK,         phk},       //$4B
{"JMP", AL, LEN3,       OP_ABS,         jmp},       //$4C
{"EOR", AL, LEN3,       OP_ABS,         eor},       //$4D
{"LSR", AL, LEN3,       OP_ABS,         lsr},       //$4E
{"EOR", NB, LEN4,       OP_ABS_L,       eor},       //$4F
{"BVC", AL, LEN2,       OP_REL,         bvc},       //$50
{"EOR", AL, LEN2,       OP_ZP_IY,       eor},       //$51
{"EOR", N2, LEN2,       OP_ZP_IND,      eor},       //$52
{"EOR", N2, LEN2,       OP_SR_IY,       eor},       //$53
{"MVN", NB, LEN3,       OP_IMM,         unimp},     //$54
{"EOR", AL, LEN2,       OP_ZP_X,        eor},       //$55
{"LSR", AL, LEN2,       OP_ZP_X,        lsr},       //$56
{"EOR", AL, LEN2,       OP_ZP_IY_L,     eor},       //$57
{"CLI", AL, LEN1,       OP_NONE,        cli},       //$58
{"EOR", AL, LEN3,       OP_ABS_Y,       eor},       //$59
{"PHY", N2, LEN1,       OP_STK,         phy},       //$5A
{"TCD", NB, LEN1,       OP_NONE,        tcd},       //$5B
{"JML", NB, LEN4,       OP_IMM,         jml},       //$5C
{"EOR", AL, LEN3,       OP_ABS_X,       eor},       //$5D
{"LSR", AL, LEN3,       OP_ABS_X,       lsr},       //$5E
{"EOR", NB, LEN4,       OP_ABS_X_L,     eor},       //$5F
{"RTS", AL, LEN1,       OP_NONE,        rts},       //$60
{"ADC", AL, LEN2,       OP_ZP_XI,       adc},       //$61
{"PER", NB, LEN3,       OP_REL_L,       unimp},     //$62
{"ADC", NB, LEN2,       OP_SR,          adc},       //$63
{"STZ", N2, LEN2,       OP_ZP,          unimp},     //$64
{"ADC", AL, LEN2,       OP_ZP,          adc},       //$65
{"ROR", AL, LEN2,       OP_ZP,          ror},       //$66
{"ADC", NB, LEN2,       OP_ZP_IND_L,    adc},       //$67
{"PLA", AL, LEN1,       OP_STK,         pla},       //$68
{"ADC", AL, LEN2|M_A,   OP_IMM,         adc},       //$69
{"ROR", AL, LEN1,       OP_A,           ror},       //$6A
{"RTL", NB, LEN1,       OP_NONE,        rtl},       //$6B
{"JMP", AL, LEN3,       OP_ABS_IND,     jmp},       //$6C
{"ADC", AL, LEN3,       OP_ABS,         adc},       //$6D
{"ROR", AL, LEN3,       OP_ABS,         ror},       //$6E
{"ADC", NB, LEN4,       OP_ABS_L,       adc},       //$6F
{"BVS", AL, LEN2,       OP_REL,         bvs},       //$70
{"ADC", AL, LEN2,       OP_ZP_IY,       adc},       //$71
{"ADC", AL, LEN2,       OP_ZP_IND,      adc},       //$72
{"ADC", NB, LEN2,       OP_SR_IY,       adc},       //$73
{"STZ", N2, LEN2,       OP_ZP_X,        unimp},     //$74
{"ADC", AL, LEN2,       OP_ZP_X,        adc},       //$75
{"ROR", AL, LEN2,       OP_ZP_X,        ror},       //$76
{"ADC", NB, LEN2,       OP_ZP_IY_L,     adc},       //$77
{"SEI", AL, LEN1,       OP_NONE,        sei},       //$78
{"ADC", AL, LEN3,       OP_ABS_Y,       adc},       //$79
{"PLY", N2, LEN1,       OP_STK,         ply},       //$7A
{"TDC", NB, LEN1,       OP_NONE,        tdc},       //$7B
{"JMP", N2, LEN3,       OP_ABS_X_IND,   jmp},       //$7C
{"ADC", AL, LEN3,       OP_ABS_X,       adc},       //$7D
{"ROR", AL, LEN3,       OP_ABS_X,       ror},       //$7E
{"ADC", NB, LEN4,       OP_ABS_X_L,     adc},       //$7F
{"BRA", N2, LEN2,       OP_REL,         bra},       //$80
{"STA", AL, LEN2,       OP_ZP_XI,       sta},       //$81
{"BRL", NB, LEN3,       OP_REL_L,       brl},       //$82
{"STA", NB, LEN2,       OP_SR,          sta},       //$83
{"STY", AL, LEN2,       OP_ZP,          sty},       //$84
{"STA", AL, LEN2,       OP_ZP,          sta},       //$85
{"STX", AL, LEN2,       OP_ZP,          stx},       //$86
{"STA", NB, LEN2,       OP_ZP_IND_L,    sta},       //$87
{"DEY", AL, LEN1,       OP_NONE,        dey},       //$88
{"BIT", N2, LEN2|M_A,   OP_IMM,         bit},       //$89
{"TXA", AL, LEN1,       OP_NONE,        txa},       //$8A
{"PHB", NB, LEN1,       OP_STK,         phb},       //$8B
{"STY", AL, LEN3,       OP_ABS,         sty},       //$8C
{"STA", AL, LEN3,       OP_ABS,         sta},       //$8D
{"STX", AL, LEN3,       OP_ABS,         stx},       //$8E
{"STA", NB, LEN4,       OP_ABS_L,       sta},       //$8F
{"BCC", AL, LEN2,       OP_REL,         bcc},       //$90
{"STA", AL, LEN2,       OP_ZP_IY,       sta},       //$91
{"STA", N2, LEN2,       OP_ZP_IND,      sta},       //$92
{"STA", NB, LEN2,       OP_SR_IY,       sta},       //$93
{"STY", AL, LEN2,       OP_ZP_X,        sty},       //$94
{"STA", AL, LEN2,       OP_ZP_X,        sta},       //$95
{"STX", AL, LEN2,       OP_ZP_Y,        stx},       //$96
{"STA", NB, LEN2,       OP_ZP_IY_L,     sta},       //$97
{"TYA", AL, LEN1,       OP_NONE,        tya},       //$98
{"STA", AL, LEN3,       OP_ABS_Y,       sta},       //$99
{"TXS", AL, LEN1,       OP_NONE,        txs},       //$9A
{"TXY", NB, LEN1,       OP_NONE,        txy},       //$9B
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
{"TAY", AL, LEN1,       OP_NONE,        tay},       //$A8
{"LDA", AL, LEN2|M_A,   OP_IMM,         lda},       //$A9
{"TAX", AL, LEN1,       OP_NONE,        tax},       //$AA
{"PLB", NB, LEN1,       OP_STK,         plb},       //$AB
{"LDY", AL, LEN3,       OP_ABS,         ldy},       //$AC
{"LDA", AL, LEN3,       OP_ABS,         lda},       //$AD
{"LDX", AL, LEN3,       OP_ABS,         ldx},       //$AE
{"LDA", NB, LEN4,       OP_ABS_L,       lda},       //$AF
{"BCS", AL, LEN2,       OP_REL,         bcs},       //$B0
{"LDA", AL, LEN2,       OP_ZP_IY,       lda},       //$B1
{"LDA", N2, LEN2,       OP_ZP_IND,      lda},       //$B2
{"LDA", NB, LEN2,       OP_SR_IY,       lda},       //$B3
{"LDY", AL, LEN2,       OP_ZP_X,        ldy},       //$B4
{"LDA", AL, LEN2,       OP_ZP_X,        lda},       //$B5
{"LDX", AL, LEN2,       OP_ZP_Y,        ldx},       //$B6
{"LDA", NB, LEN2,       OP_ZP_IY_L,     lda},       //$B7
{"CLV", AL, LEN1,       OP_NONE,        clv},       //$B8
{"LDA", AL, LEN3,       OP_ABS_Y,       lda},       //$B9
{"TSX", AL, LEN1,       OP_NONE,        tsx},       //$BA
{"TYX", NB, LEN1,       OP_NONE,        tyx},       //$BB
{"LDY", AL, LEN3,       OP_ABS_X,       ldy},       //$BC
{"LDA", AL, LEN3,       OP_ABS_X,       lda},       //$BD
{"LDX", AL, LEN3,       OP_ABS_Y,       ldx},       //$BE
{"LDA", NB, LEN4,       OP_ABS_X_L,     lda},       //$BF
{"CPY", AL, LEN2|X_A,   OP_IMM,         cpy},       //$C0
{"CMP", AL, LEN2,       OP_ZP_XI,       cmp},       //$C1
{"REP", NB, LEN2,       OP_IMM,         rep},       //$C2
{"CMP", NB, LEN2,       OP_SR,          cmp},       //$C3
{"CPY", AL, LEN2,       OP_ZP,          cpy},       //$C4
{"CMP", AL, LEN2,       OP_ZP,          cmp},       //$C5
{"DEC", AL, LEN2,       OP_ZP,          dec},       //$C6
{"CMP", NB, LEN2,       OP_ZP_IND_L,    cmp},       //$C7
{"INY", AL, LEN1,       OP_NONE,        iny},       //$C8
{"CMP", AL, LEN2|M_A,   OP_IMM,         cmp},       //$C9
{"DEX", AL, LEN1,       OP_NONE,        dex},       //$CA
{"WAI", N2, LEN1,       OP_NONE,        unimp},     //$CB
{"CPY", AL, LEN3,       OP_ABS,         cpy},       //$CC
{"CMP", AL, LEN3,       OP_ABS,         cmp},       //$CD
{"DEC", AL, LEN3,       OP_ABS,         dec},       //$CE
{"CMP", NB, LEN4,       OP_ABS_L,       cmp},       //$CF
{"BNE", AL, LEN2,       OP_REL,         bne},       //$D0
{"CMP", AL, LEN2,       OP_ZP_IY,       cmp},       //$D1
{"CMP", N2, LEN2,       OP_ZP_IND,      cmp},       //$D2
{"CMP", NB, LEN2,       OP_SR_IY,       cmp},       //$D3
{"PEI", NB, LEN2,       OP_ZP,          unimp},     //$D4
{"CMP", AL, LEN2,       OP_ZP_X,        cmp},       //$D5
{"DEC", AL, LEN2,       OP_ZP_X,        dec},       //$D6
{"CMP", NB, LEN2,       OP_ZP_IY_L,     cmp},       //$D7
{"CLD", AL, LEN1,       OP_NONE,        cld},       //$D8
{"CMP", AL, LEN3,       OP_ABS_Y,       cmp},       //$D9
{"PHX", N2, LEN1,       OP_STK,         phx},       //$DA
{"STP", N2, LEN1,       OP_NONE,        stp},       //$DB
{"JML", NB, LEN3,       OP_ABS_IND_L,   jml},       //$DC
{"CMP", AL, LEN3,       OP_ABS_X,       cmp},       //$DD
{"DEC", AL, LEN3,       OP_ABS_X,       dec},       //$DE
{"CMP", NB, LEN4,       OP_ABS_X_L,     cmp},       //$DF
{"CPX", AL, LEN2|X_A,   OP_IMM,         cpx},       //$E0
{"SBC", AL, LEN2,       OP_ZP_XI,       sbc},       //$E1
{"SEP", NB, LEN2,       OP_IMM,         sep},       //$E2
{"SBC", NB, LEN2,       OP_SR,          sbc},       //$E3
{"CPX", AL, LEN2,       OP_ZP,          cpx},       //$E4
{"SBC", AL, LEN2,       OP_ZP,          sbc},       //$E5
{"INC", AL, LEN2,       OP_ZP,          inc},       //$E6
{"SBC", NB, LEN2,       OP_ZP_IND_L,    sbc},       //$E7
{"INX", AL, LEN1,       OP_NONE,        inx},       //$E8
{"SBC", AL, LEN2|M_A,   OP_IMM,         sbc},       //$E9
{"NOP", AL, LEN1,       OP_NONE,        nop},       //$EA
{"XBA", NB, LEN1,       OP_NONE,        unimp},     //$EB
{"INC", AL, LEN3,       OP_ABS,         inc},       //$EC
{"SBC", AL, LEN3,       OP_ABS,         sbc},       //$ED
{"INC", AL, LEN3,       OP_ABS,         inc},       //$EE
{"SBC", NB, LEN4,       OP_ABS_L,       sbc},       //$EF
{"BEQ", AL, LEN2,       OP_REL,         beq},       //$F0
{"SBC", AL, LEN2,       OP_ZP_IY,       sbc},       //$F1
{"SBC", N2, LEN2,       OP_ZP_IND,      sbc},       //$F2
{"SBC", NB, LEN2,       OP_SR_IY,       sbc},       //$F3
{"PEA", NB, LEN3,       OP_IMM,         unimp},     //$F4
{"SBC", AL, LEN2,       OP_ZP_X,        sbc},       //$F5
{"INC", AL, LEN2,       OP_ZP_X,        inc},       //$F6
{"SBC", NB, LEN2,       OP_ZP_IY_L,     sbc},       //$F7
{"SED", AL, LEN1,       OP_NONE,        sed},       //$F8
{"SBC", AL, LEN3,       OP_ABS_Y,       sbc},       //$F9
{"PLX", N2, LEN1,       OP_STK,         plx},       //$FA
{"XCE", NB, LEN1,       OP_NONE,        xce},       //$FB
{"JSR", NB, LEN3,       OP_ABS_X_IND,   jsr},       //$FC
{"SBC", AL, LEN3,       OP_ABS_X,       sbc},       //$FD
{"INC", AL, LEN3,       OP_ABS_X,       inc},       //$FE
{"SBC", NB, LEN4,       OP_ABS_X_L,     sbc}        //$FF
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
        exit(0);
    }
    if ((get_cpu_type() == 2) && (unsupport & NOT_65C02)) {
        printf("Unimplemented CMOS 65c02 opcode $%02X\n", op);
        exit(0);    // Not implemented!
    }
    oplen = sizeinfo & 0x7;  // Extract length bits
   
    if (sizeinfo & M_A) {
    // Instruction:  add 1 byte if M flag is set
        if (GET_MSIZE() == 0) {
            ++oplen;
        }
    }
    if (sizeinfo & X_A)  {
        if (GET_XSIZE() == 0) {
            ++oplen;
        }
    }
    return oplen;
}


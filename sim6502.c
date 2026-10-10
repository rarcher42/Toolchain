// Starting over, with one big ugly file for code to start with.
// Will obliterate and modularize old code as development progresses
// Phase 1:  8 bit 65c02 and 6502 only, with full debugging capability
// but tables set up for full 65c816 evolution
//
// R. Archer 10/8/26
//
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE    (65536)
uint8_t mem[MEMORY_SIZE];

// Op-codes (modularize!)
void nop (void)
{

}

void clc (void)
{
}

void sec (void)
{
}

void cld (void)
{
}

void sed (void)
{
}

void sei (void)
{
}

void cli (void)
{
}

void clv (void)
{
}

void sep (void)
{
}

void rep (void)
{
}

void inx (void)
{
}

void iny (void)
{
}

void dex (void)
{
}

void dey (void)
{
}

void stp (void)
{
}

void brak (void)
{
}

void unimp (void)
{
}

void xce (void)
{
}

// Branches and Jumps
void bra (void)
{
}

void brl (void)
{
}

void bcs (void)
{
}

void bcc (void)
{
}

void bne (void)
{
}

void beq (void)
{
}

void bmi (void)
{
}

void bpl (void)
{
}

void bvs (void)
{
}

void bvc (void)
{
}

void jmp (void)
{
}

void jsr (void)
{
}

void jsl (void)
{
}

void rts (void)
{
}

void rti (void)
{
}

void rtl (void)
{
}

void jml (void)
{
}

void tax (void)
{
}

void tay (void)
{
}

void tcd (void)
{
}

void tcs (void)
{
}

void tdc (void)
{
}

void tsc (void)
{
}

void tsx (void)
{
}

void txs (void)
{
}


void txy (void)
{
}

void tyx (void)
{
}

void txa (void)
{
}

void tya (void)
{
}



void phb (void)
{
}

void phk (void)
{
}

void phd (void)
{
}

void php (void)
{
}

void pha (void)
{
}

void phx (void)
{
}

void phy (void)
{
}

void pla (void)
{
}

void plb (void)
{
}

void pld (void)
{
}

void plp (void)
{
}

void plx(void)
{
}

void ply (void)
{
}

void plk (void)
{
}

void cpx (void)
{
}

void cpy (void)
{
}

void cmp (void)
{
}

void xba (void)
{
}

void lda (void)
{
}

void ldx (void)
{
}

void ldy (void)
{
}

void tsb (void)
{
}

void trb (void)
{
}

void sta (void)
{
}

void stz (void)
{
}

void stx (void)
{
}

void sty (void)
{
}


void anl (void)
{
}


void bit (void)
{
}


void eor (void)
{
}

void ora (void)
{
}




void adc (void)
{
}

void sbc (void)
{
}

// Unlike ADC, which always uses the accumulator for the destination,
// regardless of the operand source, this instruction has the same
// destination and source.  Thus, if the source is the accumulator,
// so is the destination.  If the source is memory, then the destination
// is the same memory.  Instructions like this are:
// ASL, ROL, ROR, LSR, DEC, INC.
// ASL A, ROL A, ROR A, LSR A, DEC A, INC A operate entirely in A
// and all other variants are read/modify/write to memory

void inc (void)
{
}

void dec (void)
{
}


void lsr (void)
{
}

void asl (void)
{
}

void rol (void)
{
}

void ror (void)
{
}

// SREC loader
const int MAX_LINE = 130;

uint8_t cpu_read (uint32_t address)
{
    if (address < MEMORY_SIZE)
        return mem[address];
    else {
        printf("\nInvalid memory address $%08X", address);
        return 0;
    }
}

void cpu_write (uint32_t address, uint8_t val)
{
    if (address < MEMORY_SIZE) {
        mem[address] = val;
    } else {
        printf("\nInvalid memory address $%08X", address);
        return;
    }
}

static inline uint8_t hex_val (uint8_t c)
{
    uint8_t g = 0;

    if ((c >= '0') && (c <= '9')) {
        g = c - '0';
    } else if ((c >= 'A') && (c <= 'F')) {
        g = c - '0' - 0x7;
    } else if ((c >= 'a') && (c <= 'f')) {
        g = c - '0' - 0x27;
    }
    return g;
}

uint32_t from_hex_str (uint8_t *s, uint8_t n)
{
    uint32_t sum = 0;
    int i;

    for (i = 0; i < n; i++) {
        sum = (sum << 4) + hex_val(s[i]);
    }
    return sum;
}

uint32_t from_hex (uint32_t addr, uint8_t n)
{
    uint32_t sum = 0;
    int i;

    for (i = n-1; i >= 0; --i) {
        // printf("_%02X_", numseq[i]);
        sum = (sum << 8) + cpu_read(addr+i);
    }
    return sum;
}

void dump_hex (uint32_t sa, uint32_t ea)
{
    uint32_t addr;

    for (addr = sa; addr <= ea; addr++) {
        if ((addr & 0xF) == 0) {
            printf("\r\n%08X: ", addr);
        }
        printf("%02X ", cpu_read(addr));
    }
    printf("\r\n");
}

int load_srec (char *fn, uint32_t *start_address, uint32_t *end_address)
{
    int j;
    FILE *fp;
    char rectype;
    char nextline[MAX_LINE];
    uint32_t addr;
    uint8_t nbytes;
    uint8_t b;

    *start_address = 0xFFFFFFFF;
    *end_address = 0;
    addr = 0;
    fp = fopen(fn, "r");
    if (fp != NULL) {
        while (fgets(nextline, MAX_LINE, fp)) {
            ; // printf("%s\r", nextline);
            if (nextline[0] != 'S') {
                return -1;
            }
            rectype = nextline[1];
            switch (rectype) {
                case '0':
                    break;
                case '1':
                    nbytes = from_hex_str((uint8_t *) &nextline[2], 2) - 3;
                    addr = from_hex_str((uint8_t *) &nextline[4], 4);
                    j = 8;
                    while (nbytes-- > 0) {
                        b = from_hex_str((uint8_t *) &nextline[j], 2);
                        cpu_write(addr, b);
                        if (addr < *start_address)
                            *start_address = addr;
                        if (addr > *end_address)
                            *end_address = addr;
                        j += 2;
                        addr += 1;
                    }
                    break;
                case '2':
                    nbytes = from_hex_str((uint8_t *) &nextline[2], 2) - 4;
                    addr = from_hex_str((uint8_t *) &nextline[4], 6);
                    j = 10;
                    while (nbytes-- > 0) {
                        b = from_hex_str((uint8_t *) &nextline[j], 2);
                        cpu_write(addr, b);
                        if (addr < *start_address)
                            *start_address = addr;
                        if (addr > *end_address)
                            *end_address = addr;
                        j += 2;
                        addr += 1;
                    }
                    break;
                case '3':
                    nbytes = from_hex_str((uint8_t *) &nextline[2], 2) - 5;
                    addr = from_hex_str((uint8_t *) &nextline[4], 8);
                    j = 12;
                    while (nbytes-- > 0) {
                        b = from_hex_str((uint8_t *) &nextline[j], 2);
                        cpu_write(addr, b);
                        if (addr < *start_address)
                            *start_address = addr;
                        if (addr > *end_address)
                            *end_address = addr;
                        j += 2;
                        addr += 1;
                    }
                    break;
                case '5':
                    break;
                case '7':
                    break;
                case '8':
                    break;
                case '9':
                    break;
                default:
                    printf("UNKNOWN Record type -  discarding!");
                    break;
            }
        }

    } else {
        return -1;
    }
    dump_hex(*start_address, *end_address);
    return 0;
}


// CPU op table stuff
#define MAX_OPS_LEN (3)

// sizeinfo field: includes modifiers for M and X flags
const uint8_t FETCH1 = 0x01;
const uint8_t FETCH2 = 0x02;
const uint8_t FETCH3 = 0x03;
const uint8_t FETCH4 = 0x04;    // 65C816 only
const uint8_t M_A = 0x80;    // If M flag is 0, add 1 to ilen
const uint8_t X_A = 0x40;    // If X = 0, then add 1 to inst len

// CPU support flags
// 65816 65c02 NMOS6502
// AL = 111     Instruction supported on all 65xx
// N2 = 110     Supported on 65c816, 65c02, not supported on NMOS 6502
// NB = 100     Supported only on 65c816
// UN = 000     Unsupported instruction

const uint8_t M6502 = 0x01;     // Flag position 0
const uint8_t M65C02 = 0x02;    // Flag position 1
const uint8_t M65C816 = 0x04;   // Flag position 2

const uint8_t AL = 0x7;
const uint8_t N2 = 0x6;     // Only 6502 not supported
const uint8_t NB = 0x4;     // Only 65816 supported
const uint8_t NS = 0x0;     // Instruction not supported (invalid op-code)


typedef enum
{
    OP_NONE = 0, OP_A, OP_IMM, OP_ABS, OP_ABS_L, OP_ABS_IND, OP_ABS_IND_L,
    OP_ABS_X, OP_ABS_Y, OP_ABS_X_L, OP_ABS_X_IND, OP_ZP,
    OP_ZP_IND, OP_ZP_IND_L, OP_ZP_X, OP_ZP_Y, OP_ZP_XI,
    OP_ZP_IY, OP_ZP_IY_L, OP_REL, OP_REL_L, OP_SR, OP_SR_IY,
    OP_STK
} address_mode_t;

typedef struct op_tbl
{
    char ops[MAX_OPS_LEN+1];    // Hold full opcode plus null
    uint8_t support_flags;      // Flags for unsupported CPU versions
    uint8_t size;
    address_mode_t adm;
    void (*do_op)(void);	// Do the opcode!

} op_tbl;

// Global messiness
uint8_t cpu_type = M6502;   // Default to NMOS 6502 unless configured otherwise

void set_cputype (uint8_t cpu_type)
{
    cpu_type = cpu_type;
}

uint8_t get_cpu_type (void)
{
    return cpu_type;
}

op_tbl opcode_table[] = {
    {"BRK", AL, FETCH1,       OP_NONE,        brak},    //$00
    {"ORA", AL, FETCH2,       OP_ZP_XI,       ora},     //$01
    {"COP", NB, FETCH2,       OP_IMM,         unimp},   //$02
    {"ORA", NB, FETCH2,       OP_SR,          ora},     //$03
    {"TSB", N2, FETCH2,       OP_ZP,          tsb},     //$04
    {"ORA", AL, FETCH2,       OP_ZP,          ora},     //$05
    {"ASL", AL, FETCH2,       OP_ZP,          asl},     //$06
    {"ORA", NB, FETCH2,       OP_ZP_IND_L,    ora},     //$07
    {"PHP", AL, FETCH1,       OP_STK,         php},     //$08
    {"ORA", AL, FETCH2|M_A,   OP_IMM,         ora},     //$09
    {"ASL", AL, FETCH1,       OP_A,           asl},     //$0A
    {"PHD", NB, FETCH1,       OP_STK,         phd},     //$0B
    {"TSB", N2, FETCH3,       OP_ABS,         tsb},     //$0C
    {"ORA", AL, FETCH3,       OP_ABS,         ora},     //$0D
    {"ASL", AL, FETCH3,       OP_ABS,         asl},     //$0E
    {"ORA", NB, FETCH4,       OP_ABS_L,       ora},     //$0F
    {"BPL", AL, FETCH2,       OP_REL,         bpl},     //$10
    {"ORA", AL, FETCH2,       OP_ZP_IY,       ora},     //$11
    {"ORA", N2, FETCH2,       OP_ZP_IND,      ora},     //$12
    {"ORA", NB, FETCH2,       OP_SR_IY,       ora},     //$13
    {"TRB", N2, FETCH2,       OP_ZP,          trb},     //$14
    {"ORA", AL, FETCH2,       OP_ZP_X,        ora},     //$15
    {"ASL", AL, FETCH2,       OP_ZP_X,        asl},     //$16
    {"ORA", NB, FETCH2,       OP_ZP_IY_L,     ora},     //$17
    {"CLC", AL, FETCH1,       OP_NONE,        clc},     //$18
    {"ORA", AL, FETCH3,       OP_ABS_Y,       ora},     //$19
    {"INC", N2, FETCH1,       OP_A,           inc},     //$1A
    {"TCS", NB, FETCH1,       OP_NONE,        tcs},     //$1B
    {"TRB", N2, FETCH3,       OP_ABS,         trb},     //$1C
    {"ORA", AL, FETCH3,       OP_ABS_X,       ora},     //$1D
    {"ASL", AL, FETCH3,       OP_ABS_X,       asl},     //$1E
    {"ORA", NB, FETCH4,       OP_ABS_X_L,     ora},     //$1F
    {"JSR", AL, FETCH3,       OP_ABS,         jsr},     //$20
    {"AND", AL, FETCH2,       OP_ZP_XI,       anl},     //$21
    {"JSL", NB, FETCH4,       OP_ABS_L,       jsl},     //$22
    {"AND", NB, FETCH2,       OP_SR,          anl},     //$23
    {"BIT", AL, FETCH2,       OP_ZP,          bit},     //$24
    {"AND", AL, FETCH2,       OP_ZP,          anl},     //$25
    {"ROL", AL, FETCH2,       OP_ZP,          rol},     //$26
    {"AND", NB, FETCH2,       OP_ZP_IND_L,    anl},     //$27
    {"PLP", AL, FETCH1,       OP_STK,         plp},     //$28
    {"AND", AL, FETCH2|M_A,   OP_IMM,         anl},     //$29
    {"ROL", AL, FETCH1,       OP_A,           rol},     //$2A
    {"PLD", NB, FETCH1,       OP_STK,         pld},     //$2B
    {"BIT", AL, FETCH3,       OP_ABS,         bit},     //$2C
    {"AND", AL, FETCH3,       OP_ABS,         anl},     //$2D
    {"ROL", AL, FETCH3,       OP_ABS,         rol},     //$2E
    {"AND", NB, FETCH4,       OP_ABS_L,       anl},     //$2F
    {"BMI", AL, FETCH2,       OP_REL,         bmi},     //$30
    {"AND", AL, FETCH2,       OP_ZP_IY,       anl},     //$31
    {"AND", N2, FETCH2,       OP_ZP_IND,      anl},     //$32
    {"AND", NB, FETCH2,       OP_SR_IY,       anl},     //$33
    {"BIT", N2, FETCH2,       OP_ZP_X,        bit},     //$34
    {"AND", AL, FETCH2,       OP_ZP_X,        anl},     //$35
    {"ROL", AL, FETCH2,       OP_ZP_X,        rol},     //$36
    {"AND", NB, FETCH2,       OP_ZP_IY_L,     anl},     //$37
    {"SEC", AL, FETCH1,       OP_NONE,        sec},     //$38
    {"AND", AL, FETCH3,       OP_ABS_Y,       anl},     //$39
    {"DEC", N2, FETCH1,       OP_A,           dec},     //$3A
    {"TSC", NB, FETCH1,       OP_NONE,        tsc},     //$3B
    {"BIT", N2, FETCH3,       OP_ABS_X,       bit},     //$3C
    {"AND", AL, FETCH3,       OP_ABS_X,       anl},     //$3D
    {"ROL", AL, FETCH3,       OP_ABS_X,       rol},     //$3E
    {"AND", N2, FETCH4,       OP_ABS_X_L,     anl},     //$3F
    {"RTI", AL, FETCH1,       OP_NONE,        rti},     //$40
    {"EOR", AL, FETCH2,       OP_ZP_XI,       eor},     //$41
    {"WDM", NB, FETCH2,       OP_IMM,         unimp},   //$42
    {"EOR", NB, FETCH2,       OP_SR,          eor},     //$43
    {"MVP", NB, FETCH3,       OP_IMM,         unimp},   //$44
    {"EOR", AL, FETCH2,       OP_ZP,          eor},     //$45
    {"LSR", AL, FETCH2,       OP_ZP,          lsr},     //$46
    {"EOR", NB, FETCH2,       OP_ZP_IND_L,    eor},     //$47
    {"PHA", AL, FETCH1,       OP_STK,         pha},     //$48
    {"EOR", AL, FETCH2|M_A,   OP_IMM,         eor},     //$49
    {"LSR", AL, FETCH1,       OP_A,           lsr},     //$4A
    {"PHK", NB, FETCH1,       OP_STK,         phk},     //$4B
    {"JMP", AL, FETCH3,       OP_ABS,         jmp},     //$4C
    {"EOR", AL, FETCH3,       OP_ABS,         eor},     //$4D
    {"LSR", AL, FETCH3,       OP_ABS,         lsr},     //$4E
    {"EOR", NB, FETCH4,       OP_ABS_L,       eor},     //$4F
    {"BVC", AL, FETCH2,       OP_REL,         bvc},     //$50
    {"EOR", AL, FETCH2,       OP_ZP_IY,       eor},     //$51
    {"EOR", N2, FETCH2,       OP_ZP_IND,      eor},     //$52
    {"EOR", N2, FETCH2,       OP_SR_IY,       eor},     //$53
    {"MVN", NB, FETCH3,       OP_IMM,         unimp},   //$54
    {"EOR", AL, FETCH2,       OP_ZP_X,        eor},     //$55
    {"LSR", AL, FETCH2,       OP_ZP_X,        lsr},     //$56
    {"EOR", NB, FETCH2,       OP_ZP_IY_L,     eor},     //$57
    {"CLI", AL, FETCH1,       OP_NONE,        cli},     //$58
    {"EOR", AL, FETCH3,       OP_ABS_Y,       eor},     //$59
    {"PHY", N2, FETCH1,       OP_STK,         phy},     //$5A
    {"TCD", NB, FETCH1,       OP_NONE,        tcd},     //$5B
    {"JML", NB, FETCH4,       OP_IMM,         jml},     //$5C
    {"EOR", AL, FETCH3,       OP_ABS_X,       eor},     //$5D
    {"LSR", AL, FETCH3,       OP_ABS_X,       lsr},     //$5E
    {"EOR", NB, FETCH4,       OP_ABS_X_L,     eor},     //$5F
    {"RTS", AL, FETCH1,       OP_NONE,        rts},     //$60
    {"ADC", AL, FETCH2,       OP_ZP_XI,       adc},     //$61
    {"PER", NB, FETCH3,       OP_REL_L,       unimp},   //$62
    {"ADC", NB, FETCH2,       OP_SR,          adc},     //$63
    {"STZ", N2, FETCH2,       OP_ZP,          stz},     //$64
    {"ADC", AL, FETCH2,       OP_ZP,          adc},     //$65
    {"ROR", AL, FETCH2,       OP_ZP,          ror},     //$66
    {"ADC", NB, FETCH2,       OP_ZP_IND_L,    adc},     //$67
    {"PLA", AL, FETCH1,       OP_STK,         pla},     //$68
    {"ADC", AL, FETCH2|M_A,   OP_IMM,         adc},     //$69
    {"ROR", AL, FETCH1,       OP_A,           ror},     //$6A
    {"RTL", NB, FETCH1,       OP_NONE,        rtl},     //$6B
    {"JMP", AL, FETCH3,       OP_ABS_IND,     jmp},     //$6C
    {"ADC", AL, FETCH3,       OP_ABS,         adc},     //$6D
    {"ROR", AL, FETCH3,       OP_ABS,         ror},     //$6E
    {"ADC", NB, FETCH4,       OP_ABS_L,       adc},     //$6F
    {"BVS", AL, FETCH2,       OP_REL,         bvs},     //$70
    {"ADC", AL, FETCH2,       OP_ZP_IY,       adc},     //$71
    {"ADC", NB, FETCH2,       OP_ZP_IND,      adc},     //$72
    {"ADC", NB, FETCH2,       OP_SR_IY,       adc},     //$73
    {"STZ", N2, FETCH2,       OP_ZP_X,        stz},     //$74
    {"ADC", AL, FETCH2,       OP_ZP_X,        adc},     //$75
    {"ROR", AL, FETCH2,       OP_ZP_X,        ror},     //$76
    {"ADC", NB, FETCH2,       OP_ZP_IY_L,     adc},     //$77
    {"SEI", AL, FETCH1,       OP_NONE,        sei},     //$78
    {"ADC", AL, FETCH3,       OP_ABS_Y,       adc},     //$79
    {"PLY", N2, FETCH1,       OP_STK,         ply},     //$7A
    {"TDC", NB, FETCH1,       OP_NONE,        tdc},     //$7B
    {"JMP", N2, FETCH3,       OP_ABS_X_IND,   jmp},     //$7C
    {"ADC", AL, FETCH3,       OP_ABS_X,       adc},     //$7D
    {"ROR", AL, FETCH3,       OP_ABS_X,       ror},     //$7E
    {"ADC", NB, FETCH4,       OP_ABS_X_L,     adc},     //$7F
    {"BRA", N2, FETCH2,       OP_REL,         bra},     //$80
    {"STA", AL, FETCH2,       OP_ZP_XI,       sta},     //$81
    {"BRL", NB, FETCH3,       OP_REL_L,       brl},     //$82
    {"STA", NB, FETCH2,       OP_SR,          sta},     //$83
    {"STY", AL, FETCH2,       OP_ZP,          sty},     //$84
    {"STA", AL, FETCH2,       OP_ZP,          sta},     //$85
    {"STX", AL, FETCH2,       OP_ZP,          stx},     //$86
    {"STA", NB, FETCH2,       OP_ZP_IND_L,    sta},     //$87
    {"DEY", AL, FETCH1,       OP_NONE,        dey},     //$88
    {"BIT", N2, FETCH2|M_A,   OP_IMM,         bit},     //$89
    {"TXA", AL, FETCH1,       OP_NONE,        txa},     //$8A
    {"PHB", NB, FETCH1,       OP_STK,         phb},     //$8B
    {"STY", AL, FETCH3,       OP_ABS,         sty},     //$8C
    {"STA", AL, FETCH3,       OP_ABS,         sta},     //$8D
    {"STX", AL, FETCH3,       OP_ABS,         stx},     //$8E
    {"STA", NB, FETCH4,       OP_ABS_L,       sta},     //$8F
    {"BCC", AL, FETCH2,       OP_REL,         bcc},     //$90
    {"STA", AL, FETCH2,       OP_ZP_IY,       sta},     //$91
    {"STA", N2, FETCH2,       OP_ZP_IND,      sta},     //$92
    {"STA", NB, FETCH2,       OP_SR_IY,       sta},     //$93
    {"STY", AL, FETCH2,       OP_ZP_X,        sty},     //$94
    {"STA", AL, FETCH2,       OP_ZP_X,        sta},     //$95
    {"STX", AL, FETCH2,       OP_ZP_Y,        stx},     //$96
    {"STA", NB, FETCH2,       OP_ZP_IY_L,     sta},     //$97
    {"TYA", AL, FETCH1,       OP_NONE,        tya},     //$98
    {"STA", AL, FETCH3,       OP_ABS_Y,       sta},     //$99
    {"TXS", AL, FETCH1,       OP_NONE,        txs},     //$9A
    {"TXY", NB, FETCH1,       OP_NONE,        txy},     //$9B
    {"STZ", N2, FETCH3,       OP_ABS,         stz},     //$9C
    {"STA", AL, FETCH3,       OP_ABS_X,       sta},     //$9D
    {"STZ", N2, FETCH3,       OP_ABS_X,       stz},     //$9E
    {"STA", NB, FETCH4,       OP_ABS_X_L,     sta},     //$9F
    {"LDY", AL, FETCH2|X_A,   OP_IMM,         ldy},     //$A0
    {"LDA", AL, FETCH2,       OP_ZP_XI,       lda},     //$A1
    {"LDX", AL, FETCH2|X_A,   OP_IMM,         ldx},     //$A2
    {"LDA", NB, FETCH2,       OP_SR,          lda},     //$A3
    {"LDY", AL, FETCH2,       OP_ZP,          ldy},     //$A4
    {"LDA", AL, FETCH2,       OP_ZP,          lda},     //$A5
    {"LDX", AL, FETCH2,       OP_ZP,          ldx},     //$A6
    {"LDA", NB, FETCH2,       OP_ZP_IND_L,    lda},     //$A7
    {"TAY", AL, FETCH1,       OP_NONE,        tay},     //$A8
    {"LDA", AL, FETCH2|M_A,   OP_IMM,         lda},     //$A9
    {"TAX", AL, FETCH1,       OP_NONE,        tax},     //$AA
    {"PLB", NB, FETCH1,       OP_STK,         plb},     //$AB
    {"LDY", AL, FETCH3,       OP_ABS,         ldy},     //$AC
    {"LDA", AL, FETCH3,       OP_ABS,         lda},     //$AD
    {"LDX", AL, FETCH3,       OP_ABS,         ldx},     //$AE
    {"LDA", NB, FETCH4,       OP_ABS_L,       lda},     //$AF
    {"BCS", AL, FETCH2,       OP_REL,         bcs},     //$B0
    {"LDA", AL, FETCH2,       OP_ZP_IY,       lda},     //$B1
    {"LDA", N2, FETCH2,       OP_ZP_IND,      lda},     //$B2
    {"LDA", NB, FETCH2,       OP_SR_IY,       lda},     //$B3
    {"LDY", AL, FETCH2,       OP_ZP_X,        ldy},     //$B4
    {"LDA", AL, FETCH2,       OP_ZP_X,        lda},     //$B5
    {"LDX", AL, FETCH2,       OP_ZP_Y,        ldx},     //$B6
    {"LDA", NB, FETCH2,       OP_ZP_IY_L,     lda},     //$B7
    {"CLV", AL, FETCH1,       OP_NONE,        clv},     //$B8
    {"LDA", AL, FETCH3,       OP_ABS_Y,       lda},     //$B9
    {"TSX", AL, FETCH1,       OP_NONE,        tsx},     //$BA
    {"TYX", NB, FETCH1,       OP_NONE,        tyx},     //$BB
    {"LDY", AL, FETCH3,       OP_ABS_X,       ldy},     //$BC
    {"LDA", AL, FETCH3,       OP_ABS_X,       lda},     //$BD
    {"LDX", AL, FETCH3,       OP_ABS_Y,       ldx},     //$BE
    {"LDA", NB, FETCH4,       OP_ABS_X_L,     lda},     //$BF
    {"CPY", AL, FETCH2|X_A,   OP_IMM,         cpy},     //$C0
    {"CMP", AL, FETCH2,       OP_ZP_XI,       cmp},     //$C1
    {"REP", NB, FETCH2,       OP_IMM,         rep},     //$C2
    {"CMP", NB, FETCH2,       OP_SR,          cmp},     //$C3
    {"CPY", AL, FETCH2,       OP_ZP,          cpy},     //$C4
    {"CMP", AL, FETCH2,       OP_ZP,          cmp},     //$C5
    {"DEC", AL, FETCH2,       OP_ZP,          dec},     //$C6
    {"CMP", NB, FETCH2,       OP_ZP_IND_L,    cmp},     //$C7
    {"INY", AL, FETCH1,       OP_NONE,        iny},     //$C8
    {"CMP", AL, FETCH2|M_A,   OP_IMM,         cmp},     //$C9
    {"DEX", AL, FETCH1,       OP_NONE,        dex},     //$CA
    {"WAI", N2, FETCH1,       OP_NONE,        unimp},   //$CB
    {"CPY", AL, FETCH3,       OP_ABS,         cpy},     //$CC
    {"CMP", AL, FETCH3,       OP_ABS,         cmp},     //$CD
    {"DEC", AL, FETCH3,       OP_ABS,         dec},     //$CE
    {"CMP", NB, FETCH4,       OP_ABS_L,       cmp},     //$CF
    {"BNE", AL, FETCH2,       OP_REL,         bne},     //$D0
    {"CMP", AL, FETCH2,       OP_ZP_IY,       cmp},     //$D1
    {"CMP", N2, FETCH2,       OP_ZP_IND,      cmp},     //$D2
    {"CMP", NB, FETCH2,       OP_SR_IY,       cmp},     //$D3
    {"PEI", NB, FETCH2,       OP_ZP,          unimp},   //$D4
    {"CMP", AL, FETCH2,       OP_ZP_X,        cmp},     //$D5
    {"DEC", AL, FETCH2,       OP_ZP_X,        dec},     //$D6
    {"CMP", NB, FETCH2,       OP_ZP_IY_L,     cmp},     //$D7
    {"CLD", AL, FETCH1,       OP_NONE,        cld},     //$D8
    {"CMP", AL, FETCH3,       OP_ABS_Y,       cmp},     //$D9
    {"PHX", N2, FETCH1,       OP_STK,         phx},     //$DA
    {"STP", N2, FETCH1,       OP_NONE,        stp},     //$DB
    {"JML", NB, FETCH3,       OP_ABS_IND_L,   jml},     //$DC
    {"CMP", AL, FETCH3,       OP_ABS_X,       cmp},     //$DD
    {"DEC", AL, FETCH3,       OP_ABS_X,       dec},     //$DE
    {"CMP", NB, FETCH4,       OP_ABS_X_L,     cmp},     //$DF
    {"CPX", AL, FETCH2|X_A,   OP_IMM,         cpx},     //$E0
    {"SBC", AL, FETCH2,       OP_ZP_XI,       sbc},     //$E1
    {"SEP", NB, FETCH2,       OP_IMM,         sep},     //$E2
    {"SBC", NB, FETCH2,       OP_SR,          sbc},     //$E3
    {"CPX", AL, FETCH2,       OP_ZP,          cpx},     //$E4
    {"SBC", AL, FETCH2,       OP_ZP,          sbc},     //$E5
    {"INC", AL, FETCH2,       OP_ZP,          inc},     //$E6
    {"SBC", NB, FETCH2,       OP_ZP_IND_L,    sbc},     //$E7
    {"INX", AL, FETCH1,       OP_NONE,        inx},     //$E8
    {"SBC", AL, FETCH2|M_A,   OP_IMM,         sbc},     //$E9
    {"NOP", AL, FETCH1,       OP_NONE,        nop},     //$EA
    {"XBA", NB, FETCH1,       OP_NONE,        xba},     //$EB
    {"INC", AL, FETCH3,       OP_ABS,         inc},     //$EC
    {"SBC", AL, FETCH3,       OP_ABS,         sbc},     //$ED
    {"INC", AL, FETCH3,       OP_ABS,         inc},     //$EE
    {"SBC", NB, FETCH4,       OP_ABS_L,       sbc},     //$EF
    {"BEQ", AL, FETCH2,       OP_REL,         beq},     //$F0
    {"SBC", AL, FETCH2,       OP_ZP_IY,       sbc},     //$F1
    {"SBC", N2, FETCH2,       OP_ZP_IND,      sbc},     //$F2
    {"SBC", NB, FETCH2,       OP_SR_IY,       sbc},     //$F3
    {"PEA", NB, FETCH3,       OP_IMM,         unimp},   //$F4
    {"SBC", AL, FETCH2,       OP_ZP_X,        sbc},     //$F5
    {"INC", AL, FETCH2,       OP_ZP_X,        inc},     //$F6
    {"SBC", NB, FETCH2,       OP_ZP_IY_L,     sbc},     //$F7
    {"SED", AL, FETCH1,       OP_NONE,        sed},     //$F8
    {"SBC", AL, FETCH3,       OP_ABS_Y,       sbc},     //$F9
    {"PLX", N2, FETCH1,       OP_STK,         plx},     //$FA
    {"XCE", NB, FETCH1,       OP_NONE,        xce},     //$FB
    {"JSR", NB, FETCH3,       OP_ABS_X_IND,   jsr},     //$FC
    {"SBC", AL, FETCH3,       OP_ABS_X,       sbc},     //$FD
    {"INC", AL, FETCH3,       OP_ABS_X,       inc},     //$FE
    {"SBC", NB, FETCH4,       OP_ABS_X_L,     sbc}      //$FF
};

char *get_mnemonic (uint8_t op)
{
    return opcode_table[op].ops;
}

uint8_t fetch_len(uint8_t op)
{
    return opcode_table[op].size & 0x3;    // Ignore M and X flags for now; 8 bit modes
}

void *get_op_function(uint8_t op)
{
    return opcode_table[op].do_op;
}

address_mode_t get_addr_mode (uint8_t op)
{
    return opcode_table[op].adm;
}

typedef struct cpu_context
{
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S;
    uint16_t PC;
    uint8_t P;
    // Pseudo-registers
    uint8_t oplen;  // Number of valid ir[] bytes
    uint8_t ir[4];  // Instruction register (pseudo-registers)
} cpu_reg_t;

#define N_FLAG 0x80
#define V_FLAG 0x40
#define B_FLAG 0x10
#define D_FLAG 0x08
#define I_FLAG 0x04
#define Z_FLAG 0x02
#define C_FLAG 0x01


cpu_reg_t cpu_reg;

void dump_registers (void)
{
    uint8_t f;
    char flagstr[9] = "        ";

    f = cpu_reg.P;
    if (f & N_FLAG)
        flagstr[0] = 'N';
    if (f & V_FLAG)
        flagstr[1] = 'V';
    flagstr[2] = '-';
    if (f & B_FLAG)
        flagstr[3] = 'B';
    if (f & D_FLAG)
        flagstr[4] = 'D';
    if (f & I_FLAG)
        flagstr[5] = 'I';
    if (f & Z_FLAG)
        flagstr[6] = 'Z';
    if (f & C_FLAG)
        flagstr[7] = 'C';

    printf("PC=$%04X A=$%02X X=$%02X Y=$%02X SP=$01%02X P=%s\n", cpu_reg.PC, cpu_reg.A, cpu_reg.X,                    cpu_reg.Y, cpu_reg.S, flagstr);
}

void disasm (uint32_t start, uint32_t end)
{
    uint32_t vpc;
    uint8_t fetchlen;
    int i;

    vpc = start;
    while (vpc <= end) {
        fetchlen = fetch_len(mem[vpc]);
        printf("%s  ", get_mnemonic(mem[vpc]));
        for (i = 0; i < fetchlen; i++) {
            printf(" %02X", mem[vpc+i]);
        }
        printf("\n");
        vpc += fetchlen;
    }
}

void print_ir (void)
{
    int i;

    printf("$%04X: ", cpu_reg.PC);
    for (i = 0; i < cpu_reg.oplen; i++) {
        printf("%02X ", cpu_reg.ir[i]);
    }
    printf("\n");
}

void fetch (void)
{
    int i;
    uint16_t ip;
    uint8_t opcode;

    ip = cpu_reg.PC;
    opcode = mem[ip];
    cpu_reg.oplen = fetch_len(opcode);
    for (i = 0; i < cpu_reg.oplen; i++) {
        cpu_reg.ir[i] = mem[ip + i];
    }
    print_ir();
    cpu_reg.PC += cpu_reg.oplen;
}

int main (void)
{
    uint32_t start_address;
    uint32_t end_address;
    int i;

    set_cputype(M65C02);
    load_srec("dow.hex", &start_address, &end_address);
    printf("\nStart address: $%08X", start_address);
    printf("\n  End address: $%08X", end_address);
    cpu_reg.PC = start_address;
    dump_registers();
    // disasm(start_address, end_address);
    for (i = 0; i < 50; i++)
        fetch();
    printf("\n\n\nCPU simulator done!\n");

}

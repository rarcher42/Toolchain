#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sim6502.h"
#include "sim_opcodes.h"
#include "sim_computer.h"

static inline uint8_t get_A (cpu_context_t *cpu)
{
    return cpu->reg.A;
}

static void put_A (cpu_context_t *cpu, uint8_t val)
{
    cpu->reg.A = val;
}

static inline uint8_t get_X (cpu_context_t *cpu)
{
    return cpu->reg.X;
}

static void put_X (cpu_context_t *cpu, uint8_t val)
{
    cpu->reg.X = val;
}

static inline uint8_t get_Y (cpu_context_t *cpu)
{
    return cpu->reg.Y;
}

static void put_Y (cpu_context_t *cpu, uint8_t val)
{
    cpu->reg.Y = val;
}

static inline uint8_t get_raw_S (cpu_context_t *cpu)
{
    return cpu->reg.S;
}

static void put_raw_S (cpu_context_t *cpu, uint8_t val)
{
    cpu->reg.S = val;
}

static inline uint8_t get_P (cpu_context_t *cpu)
{
    return cpu->reg.P;
}

static void put_P (cpu_context_t *cpu, uint8_t val)
{
    cpu->reg.P = val;
}

static inline uint16_t get_PC (cpu_context_t *cpu)
{
    return cpu->reg.PC;
}

static void put_PC (cpu_context_t *cpu, uint16_t val)
{
    cpu->reg.PC = val;
}

void change_zflag(cpu_context_t *cpu, uint16_t val)
{
    if (val == 0) {
        set_flag(cpu, Z_FLAG);
    } else {
        clr_flag(cpu, Z_FLAG);
    }
}

// Many instructions set N and Z flag by value.
// N is set the MSB of result,
// Z is set by whether the result is zero
void change_nzflag(cpu_context_t *cpu, uint8_t val)
{
    change_zflag(cpu, val);

    if ((val & 0x80) == 0) {
        clr_flag(cpu, N_FLAG);
    } else {
        set_flag(cpu, N_FLAG);
    }
}

static void push8 (cpu_context_t *cpu, uint8_t val)
{
    uint16_t sp;

    sp = (get_raw_S(cpu) | 0x100) & 0x1FF;
    cpu_write(cpu, sp, val);
    put_raw_S(cpu, (--sp & 0xFF));
}

static void push16 (cpu_context_t *cpu, uint16_t val)
{
    push8(cpu, (val >> 8) & 0xFF);
    push8(cpu, val & 0xFF);
}

uint8_t pop8 (cpu_context_t *cpu)
{
    uint16_t sp;
    uint8_t val;

    sp = (get_raw_S(cpu) + 1) | 0x100;
    put_raw_S(cpu, sp & 0xFF);
    val = cpu_read(cpu, sp);
    return val;
}

uint16_t pop16 (cpu_context_t *cpu)
{
    uint16_t val;
    uint8_t lsb;
    lsb = pop8(cpu);
    val = (pop8(cpu) << 8) | lsb;
    return val;
}

void nop (cpu_context_t *cpu)
{
    // Add cycle counter
}

void clc (cpu_context_t *cpu)
{
    clr_flag(cpu, C_FLAG);
}

void sec (cpu_context_t *cpu)
{
    set_flag(cpu, C_FLAG);
}

void cld (cpu_context_t *cpu)
{
    clr_flag(cpu, D_FLAG);
}

void sed (cpu_context_t *cpu)
{
    set_flag(cpu, D_FLAG);
}

void sei (cpu_context_t *cpu)
{
    set_flag(cpu, I_FLAG);
}

void cli (cpu_context_t *cpu)
{
    clr_flag(cpu, I_FLAG);
}

void clv (cpu_context_t *cpu)
{
    clr_flag(cpu, V_FLAG);
}

/* Note:  there is no set overflow instruction */

void sep (cpu_context_t *cpu)
{
    printf("\nNot implemented!");
    exit(0);
}

void rep (cpu_context_t *cpu)
{
    printf("\nNot implemented!");
    exit(0);
}

void dex (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = (get_X(cpu) - 1);
    change_nzflag(cpu, temp);
    put_X(cpu, temp);
}

void dey (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = (get_Y(cpu) - 1);
    change_nzflag(cpu, temp);
    put_Y(cpu, temp);
}


void inx (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_X(cpu) + 1;
    change_nzflag(cpu, temp);
    put_X(cpu, temp);
}

void iny (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_Y(cpu) + 1;
    change_nzflag(cpu, temp);
    put_Y(cpu, temp);
}

void phb (cpu_context_t *cpu)
{
    // push8(cpu_state.DBR);
}

void phk (cpu_context_t *cpu)
{
    // push8(cpu_state.PBR);
}

void phd (cpu_context_t *cpu)
{
    // push16(cpu_state.DPR);
}

void php (cpu_context_t *cpu)
{
    push8(cpu, get_P(cpu));
}

void pha (cpu_context_t *cpu)
{
    push8(cpu, get_A(cpu));
}

void phx (cpu_context_t *cpu)
{
}

void phy (cpu_context_t *cpu)
{
}

void pla (cpu_context_t *cpu)
{
    uint8_t vb;

    vb = pop8(cpu);
    put_A(cpu, vb);
    change_nzflag(cpu, vb);
}

void plb (cpu_context_t *cpu)
{
}

void pld (cpu_context_t *cpu)
{
}

void plp (cpu_context_t *cpu)
{
    uint8_t vb;

    vb = pop8(cpu);
    put_P(cpu, vb);
    // Don't set NV flags - they remain exactly as pulled from stack!
}

void plx(cpu_context_t *cpu)
{
}

void ply (cpu_context_t *cpu)
{
}

void plk (cpu_context_t *cpu)
{
}

void tax (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_A(cpu);
    put_X(cpu, temp);
    change_nzflag(cpu, temp);
}

void tay (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_A(cpu);
    put_Y(cpu, temp);
    change_nzflag(cpu, temp);
}

void tcd (cpu_context_t *cpu)
{
}

void tcs (cpu_context_t *cpu)
{
}

void tdc (cpu_context_t *cpu)
{
}

void tsc (cpu_context_t *cpu)
{
}

void tsx (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_raw_S(cpu);
    put_X(cpu, temp);
    change_nzflag(cpu, temp);
}

void txs (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_X(cpu);
    put_raw_S(cpu, temp);
    // No flag changes!
}


void txy (cpu_context_t *cpu)
{
}

void tyx (cpu_context_t *cpu)
{
}

void txa (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_X(cpu);
    put_A(cpu, temp);
    change_nzflag(cpu, temp);
}

void tya (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = get_Y(cpu);
    put_A(cpu, temp);
    change_nzflag(cpu, temp);
}

void bcs (cpu_context_t *cpu)
{
    if (get_flag(cpu, C_FLAG)) {
        put_PC(cpu, get_EA(cpu));
    }
}

void bcc (cpu_context_t *cpu)
{
    if (get_flag(cpu, C_FLAG) == 0) {
        put_PC(cpu, get_EA(cpu));
    }
}

void bne (cpu_context_t *cpu)
{
    if (get_flag(cpu, Z_FLAG) == 0) {
        put_PC(cpu, get_EA(cpu));
    }
}

void beq (cpu_context_t *cpu)
{
    if (get_flag(cpu, Z_FLAG)) {
        put_PC(cpu, get_EA(cpu));
    }
}

void bmi (cpu_context_t *cpu)
{
    if (get_flag(cpu, N_FLAG)) {
        put_PC(cpu, get_EA(cpu));
    }
}

void bpl (cpu_context_t *cpu)
{
    if (get_flag(cpu, N_FLAG) == 0) {
        put_PC(cpu, get_EA(cpu));
    }
}

void bvs (cpu_context_t *cpu)
{
    if (get_flag(cpu, V_FLAG)) {
        put_PC(cpu, get_EA(cpu));
    }
}

void bvc (cpu_context_t *cpu)
{
    if (get_flag(cpu, V_FLAG) == 0) {
        put_PC(cpu, get_EA(cpu));
    }
}

#if 0
void stp (cpu_context_t *cpu)
{
    printf("Unimplemented!");
    exit(0);
}

void brak (cpu_context_t *cpu)
{
    uint8_t lsb, msb;

    printf("\nBRK - STOPPING!\n");
    if (IS_EMU()) {
        push16(cpu_state.SP + 2);
        push8(cpu_state.flags | B_FLAG); // 65x02 doesn't set B flag, ONLY on stack copy!
        SET_FLAG(I_FLAG);
        lsb = cpu_read(0xFFFE);
        msb = cpu_read(0xFFFF);
        cpu_state.PC = (msb << 8) | lsb;
    } else {
        push8(cpu_state.PBR);
        push16(cpu_state.SP + 2);
        push8(cpu_state.flags);
        SET_FLAG(I_FLAG);
        cpu_state.PBR = 0;
        lsb = cpu_read(0xFFE6);
        msb = cpu_read(0xFFE7);
        cpu_state.PC = (msb << 8) | lsb;
    }
    cpu_dynamic_metadata.running = FALSE;
}

void unimp (cpu_context_t *cpu)
{
    printf("\nUnimplemented op-code %02X: aborting\n", get_ir_opcode());
    cpu_dynamic_metadata.running = FALSE;
}

void xce (cpu_context_t *cpu)
{
}

// Branches and Jumps
void bra (cpu_context_t *cpu)
{
    cpu_state.PC = get_EA();
}

void brl (cpu_context_t *cpu)
{
    cpu_state.PC = get_EA();
}



void jmp (cpu_context_t *cpu)
{
    cpu_state.PC = get_EA() - 3;
}

void jsr (cpu_context_t *cpu)
{
    uint16_t target;
    uint16_t ra;

    target = get_EA() - 3;  //
    ra = cpu_state.PC + 1; // 6502 family writes RA-1 to stack
    push16(ra);
    cpu_state.PC = target;
}

void jsl (cpu_context_t *cpu)
{
}

void rts (cpu_context_t *cpu)
{
    uint16_t ra;

    ra = pop16() + 1;   // Increment ra to point to next instruction
    cpu_state.PC = ra;
}

void rti (cpu_context_t *cpu)
{
    uint16_t ra;
    uint8_t flags;

    ra = pop16() + 1;   // Increment ra to point to next instruction
    flags = pop8();
    // FIXME: must handle PBR from context.  Clean up CPU context first
    cpu_state.flags = flags;
    cpu_state.PC = ra;
}

void rtl (cpu_context_t *cpu)
{
}

void jml (cpu_context_t *cpu)
{
}




void set_compare_borrow (cpu_context_t *cpu, uint32_t diff, BOOL sixteen)
{
    if (sixteen) {
        if (diff > 0xFFFF) {
            clr_flag(cpu, C_FLAG);   // A - M caused a borrow!
        } else {
            set_flag(cpu, C_FLAG);   // A - M resulted in no borrow
        }
    } else {
        if (diff > 0xFF) {
            clr_flag(cpu, C_FLAG);   // A - M caused borrow!
        } else {
            set_flag(cpu, C_FLAG);   // A - M no borrow
        }
    }
}

void cpx (cpu_context_t *cpu)
{
    uint16_t reg;
    uint16_t op;
    uint32_t diff;
    uint8_t reg_l;
    uint8_t op_l;

    if (GET_XSIZE() == 0) {
        // 16 bit compare
        reg = cpu_state.X;
        load_temp16();
        op = cpu_dynamic_metadata.TEMP;
        diff = reg - op;
        set_compare_borrow(diff, TRUE);
        change_nzflag(diff, TRUE);
    } else {
        // 8 bit compare
        reg_l = cpu_state.X & 0xFF;
        load_temp8();
        op_l = cpu_dynamic_metadata.TEMP & 0xFF;
        diff = reg_l - op_l;
        set_compare_borrow(diff, FALSE);
        change_nzflag(diff & 0xFF, FALSE);
    }
}

void cpy (cpu_context_t *cpu)
{
    uint16_t reg;
    uint16_t op;
    uint32_t diff;
    uint8_t reg_l;
    uint8_t op_l;


    if (GET_XSIZE() == 0) {
        // 16 bit compare
        reg = cpu_state.Y;
        load_temp16();
        op = cpu_dynamic_metadata.TEMP;
        diff = reg - op;
        set_compare_borrow(diff, TRUE);
        change_nzflag(diff, TRUE);
    } else {
        // 8 bit compare
        reg_l = cpu_state.Y & 0xFF;
        load_temp8();
        op_l = cpu_dynamic_metadata.TEMP & 0xFF;
        diff = reg_l - op_l;
        set_compare_borrow(diff, FALSE);
        change_nzflag(diff & 0xFF, FALSE);
    }
}

void cmp (cpu_context_t *cpu)
{
    uint16_t reg;
    uint16_t op;
    uint32_t diff;
    uint8_t reg_l;
    uint8_t op_l;

    if (GET_XSIZE() == 0) {
        // 16 bit compare
        reg = cpu_state.A.C;
        load_temp16();
        op = cpu_dynamic_metadata.TEMP;
        diff = reg - op;
        set_compare_borrow(diff, TRUE);
        change_nzflag(diff, TRUE);
    } else {
        // 8 bit compare
        reg_l = cpu_state.A.AL & 0xFF;
        load_temp8();
        op_l = cpu_dynamic_metadata.TEMP & 0xFF;
        diff = reg_l - op_l;
        set_compare_borrow(diff, FALSE);
        change_nzflag(diff & 0xFF, FALSE);
    }
}

void xba (cpu_context_t *cpu)
{
    uint8_t temp;

    temp = cpu_state.A.B;
    cpu_state.A.B = cpu_state.A.AL;
    cpu_state.A.AL = temp;
    change_nzflag(temp, FALSE);
}

void lda (cpu_context_t *cpu)
{
    if (GET_MSIZE() == 0) {
        load_temp16();
        cpu_state.A.C = cpu_dynamic_metadata.TEMP;
        change_nzflag(cpu_state.A.C, TRUE);
    } else {
        load_temp8();
        cpu_state.A.AL = cpu_dynamic_metadata.TEMP & 0xFF;
        change_nzflag(cpu_state.A.AL, FALSE);
    }

}

void ldx (cpu_context_t *cpu)
{
    if (GET_XSIZE() == 0) {
        load_temp16();
        cpu_state.X = cpu_dynamic_metadata.TEMP;
        change_nzflag(cpu_state.X, TRUE);
    } else {
        load_temp8();
        cpu_state.X = cpu_dynamic_metadata.TEMP & 0xFF;
        change_nzflag(cpu_state.X, FALSE);
    }
}

void ldy (cpu_context_t *cpu)
{
    if (GET_XSIZE() == 0) {
        load_temp16();
        cpu_state.Y = cpu_dynamic_metadata.TEMP;
        change_nzflag(cpu_state.Y, TRUE);
    } else {
        load_temp8();
        cpu_state.Y = (cpu_dynamic_metadata.TEMP & 0xFF);
        change_nzflag(cpu_state.Y, FALSE);
    }
}

void tsb (cpu_context_t *cpu)
{
    uint16_t v;
    uint8_t v_l;

    if (GET_MSIZE() == 0) {
        load_temp16();
        v = cpu_dynamic_metadata.TEMP;
        v |= cpu_state.A.C;
        cpu_dynamic_metadata.TEMP = v;
        store_temp16();
        v = v & cpu_state.A.C;
        change_zflag(v);
    } else {
        load_temp8();
        v_l = cpu_dynamic_metadata.TEMP & 0xFF;
        v_l |= cpu_state.A.AL;
        cpu_dynamic_metadata.TEMP = v_l;
        store_temp8();
        v_l = v_l & cpu_state.A.AL;
        change_zflag(v_l);
    }
}

void trb (cpu_context_t *cpu)
{
    uint16_t v;
    uint8_t v_l;

    if (GET_MSIZE() == 0) {
        load_temp16();
        v = cpu_dynamic_metadata.TEMP;
        v &= ~cpu_state.A.C;
        cpu_dynamic_metadata.TEMP = v;
        store_temp16();
        v = v & cpu_state.A.C;
        change_zflag(v);
    } else {
        load_temp8();
        v_l = cpu_dynamic_metadata.TEMP & 0xFF;
        v_l &= (~cpu_state.A.AL) & 0xFF;
        cpu_dynamic_metadata.TEMP = v_l;
        store_temp8();
        v_l = v_l & cpu_state.A.AL;
        change_zflag(v_l);
    }
}

void sta (cpu_context_t *cpu)
{
    if (GET_MSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.A.C;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.A.AL & 0xFF);
        store_temp8();
    }
}

void stz (cpu_context_t *cpu)
{
    if (GET_MSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = (uint16_t) 0;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (uint8_t) 0;
        store_temp8();
    }
}

void stx (cpu_context_t *cpu)
{
    if (GET_XSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.X;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.X & 0xFF);
        store_temp8();
    }
}

void sty (cpu_context_t *cpu)
{
    if (GET_XSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.Y;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.Y & 0xFF);
        store_temp8();
    }
}


void anl (cpu_context_t *cpu)
{
    uint16_t v1;
    uint16_t v2;
    uint16_t res;
    uint8_t v1_l;
    uint8_t v2_l;
    uint8_t res_l;


    if (GET_MSIZE() == 0) {
        // 16 bit
        v1 = cpu_state.A.C;
        load_temp16();  // Note: could be immediate data
        v2 = cpu_dynamic_metadata.TEMP;
        res = v1 & v2;
        cpu_state.A.C = res;
        change_nzflag(res, TRUE);
    } else {
        // 8 bits
        v1_l = cpu_state.A.AL;
        load_temp8();   // Note: could be immediate data
        v2_l = cpu_dynamic_metadata.TEMP & 0xFF;
        res_l = v1_l & v2_l;
        cpu_state.A.AL = res_l;
        change_nzflag(res_l, FALSE);
    }
}

// Set N to MSB, V to next-to-MSB, for BIT instruction
static void bit_nv(cpu_context_t *cpu, uint16_t v)
{
    if ((v & 0x80) == 0) {
        clr_flag(cpu, N_FLAG);
    } else {
        set_flag(cpu, N_FLAG);
    }

    if ((v & 0x40) == 0) {
        clr_flag(cpu, V_FLAG);
    } else {
        set_flag(cpu, V_FLAG);
    }
}

void bit (cpu_context_t *cpu)
{
    uint16_t v1;
    uint16_t v2;
    uint16_t res;
    uint8_t v1_l;
    uint8_t v2_l;
    uint8_t res_l;

    // Note: immmediate mode special case:  Use A instead of memory
    // read by load_temp() to set flags N&V
    if (GET_MSIZE() == 0) {
        // 16 bit
        v1 = cpu_state.A.C;
        load_temp16();  // Note: could be immediate data
        v2 = cpu_dynamic_metadata.TEMP;
        if (get_ir_addr_mode() != OP_IMM) {
            bit_nv(v2, TRUE);
        } else {
            bit_nv(v1, TRUE);   // Immediate mode special use: use A not operand
        }
        res = v1 & v2;
        cpu_state.A.C = res;
        if (res == 0) {
            SET_FLAG(Z_FLAG);
        } else {
            CLR_FLAG(Z_FLAG);
        }
    } else {
        // 8 bits
        v1_l = cpu_state.A.AL;
        // v1 (ACC) for BIT #imm
        load_temp8();   // Note: could be immediate data
        v2_l = cpu_dynamic_metadata.TEMP & 0xFF;
        if (get_ir_addr_mode() != OP_IMM) {
            bit_nv(v2_l, FALSE);
        } else {
            bit_nv(v1_l, FALSE);    // Immediate mode special use: use A not operand
        }
        res_l = v1_l & v2_l;
        cpu_state.A.AL = res_l;
        if (res_l == 0) {
            SET_FLAG(Z_FLAG);
        } else {
            CLR_FLAG(Z_FLAG);
        }
    }
}


void eor (cpu_context_t *cpu)
{
    uint16_t v1;
    uint16_t v2;
    uint16_t res;
    uint8_t v1_l;
    uint8_t v2_l;
    uint8_t res_l;


    if (GET_MSIZE() == 0) {
        // 16 bit
        v1 = cpu_state.A.C;
        load_temp16();  // Note: could be immediate data
        v2 = cpu_dynamic_metadata.TEMP;
        res = v1 ^ v2;
        cpu_state.A.C = res;
        change_nzflag(res, TRUE);
    } else {
        // 8 bits
        v1_l = cpu_state.A.AL;
        load_temp8();   // Note: could be immediate data
        v2_l = cpu_dynamic_metadata.TEMP & 0xFF;
        res_l = v1_l ^ v2_l;
        cpu_state.A.AL = res_l;
        change_nzflag(res_l, FALSE);
    }
}

void ora (cpu_context_t *cpu)
{
    uint16_t v1;
    uint16_t v2;
    uint16_t res;
    uint8_t v1_l;
    uint8_t v2_l;
    uint8_t res_l;


    if (GET_MSIZE() == 0) {
        // 16 bit
        v1 = cpu_state.A.C;
        load_temp16();  // Note: could be immediate data
        v2 = cpu_dynamic_metadata.TEMP;
        res = v1 ^ v2;
        cpu_state.A.C = res;
        change_nzflag(res, TRUE);
    } else {
        // 8 bits
        v1_l = cpu_state.A.AL;
        load_temp8();   // Note: could be immediate data
        v2_l = cpu_dynamic_metadata.TEMP & 0xFF;
        res_l = v1_l ^ v2_l;
        cpu_state.A.AL = res_l;
        change_nzflag(res_l, FALSE);
    }
}


uint8_t bcd_add4 (uint8_t x1, uint8_t y1, uint8_t c_in)
{
    uint8_t sum;
    uint8_t x, y;

    x = x1 & 0xF;
    y = y1 & 0xF;

    sum = x + y + c_in;
    if (sum > 9) {
        sum += 6;   // Decimal adjust
    }
    return sum; // Note: caller must deduce carry out, mask out b0-b3
}

uint8_t bcd_add8 (uint8_t x, uint8_t y, uint8_t c_in)
{
    int i;
    uint8_t sum;
    uint8_t result[2];  // Result nibbles here
    uint8_t digit;

    for (i = 0; i < 2; i++) {
        digit = bcd_add4(x & 0xF, y & 0xF, c_in);
        if (digit > 9) {
            c_in = 1;
        } else {
            c_in = 0;
        }
        result[i] = digit & 0xF;
        x = x >> 4;
        y = y >> 4;
    }

    for (i = 2; i > 0; --i) {
        sum = (sum << 4) | result[i-1];
    }
    if (c_in) {
        SET_FLAG(C_FLAG);
    } else {
        CLR_FLAG(C_FLAG);
    }
    return sum;
}

uint16_t bcd_add16 (uint8_t x, uint8_t y, uint8_t c_in)
{
    int i;
    uint16_t sum;
    uint8_t result[4];  // Result nibbles here
    uint8_t digit;

    for (i = 0; i < 4; i++) {
        digit = bcd_add4(x & 0xF, y & 0xF, c_in);
        if (digit > 9) {
            c_in = 1;
        } else {
            c_in = 0;
        }
        result[i] = digit & 0xF;
        x = x >> 4;
        y = y >> 4;
    }

    for (i = 4; i > 0; --i) {
        sum = (sum << 4) | result[i-1];
    }
    if (c_in) {
        SET_FLAG(C_FLAG);
    } else {
        CLR_FLAG(C_FLAG);
    }
    return sum;
}

uint8_t bcd_sub4 (uint8_t x1, uint8_t y1, uint8_t b_in)
{
    uint8_t diff;
    uint8_t x, y;

    x = x1 & 0xF;
    y = y1 & 0xF;

    diff = x - y - b_in;
    if (diff > 9) {
        diff -= 6;  // Decimal adjust
    }
    return diff;    // Note: caller must deduce carry out, mask out b0-b3
}

uint8_t bcd_sub8 (uint8_t x, uint8_t y, uint8_t c_in)
{
    int i;
    uint8_t sum;
    uint8_t result[2];  // Result nibbles here
    uint8_t digit;

    for (i = 0; i < 2; i++) {
        digit = bcd_sub4(x & 0xF, y & 0xF, c_in);
        if (digit > 9) {
            c_in = 1;
        } else {
            c_in = 0;
        }
        result[i] = digit & 0xF;
        x = x >> 4;
        y = y >> 4;
    }

    for (i = 2; i > 0; --i) {
        sum = (sum << 4) | result[i-1];
    }
    if (c_in) {
        CLR_FLAG(C_FLAG);
    } else {
        SET_FLAG(C_FLAG);
    }
    return sum;
}

uint8_t bcd_sub16 (uint8_t x, uint8_t y, uint8_t c_in)
{
    int i;
    uint16_t sum;
    uint8_t result[4];  // Result nibbles here
    uint8_t digit;

    for (i = 0; i < 4; i++) {
        digit = bcd_sub4(x & 0xF, y & 0xF, c_in);
        if (digit > 9) {
            c_in = 1;
        } else {
            c_in = 0;
        }
        result[i] = digit & 0xF;
        x = x >> 4;
        y = y >> 4;
    }

    for (i = 4; i > 0; --i) {
        sum = (sum << 4) | result[i-1];
    }
    if (c_in) {
        CLR_FLAG(C_FLAG);
    } else {
        SET_FLAG(C_FLAG);
    }
    return sum;
}

void adc (cpu_context_t *cpu)
{
    uint32_t sum;
    uint16_t v1;
    uint16_t v2;
    uint8_t c_in;

    if (GET_FLAG(C_FLAG)) {
        c_in = 1;
    } else {
        c_in = 0;
    }

    if (GET_MSIZE() == 0) {
        // 16 bit add
        v1 = cpu_state.A.C;
        load_temp16();
        v2 = cpu_dynamic_metadata.TEMP;
        if (GET_FLAG(D_FLAG)) {
            sum = bcd_add16(v1, v2, c_in);
        } else {
            sum = v1 + v2 + c_in;
            if (sum > 0xFFFF) {
                SET_FLAG(C_FLAG);
            } else {
                CLR_FLAG(C_FLAG);
            }
        }
        sum &= 0xFFFF;
        cpu_state.A.C = sum;
        change_nzflag(sum, TRUE);
        change_vflag(v1, v2, sum, TRUE);
    } else {
        // 8 bits
        v1 = cpu_state.A.AL;
        load_temp8();
        v2 = cpu_dynamic_metadata.TEMP & 0xFF;
        if (GET_FLAG(D_FLAG)) {
            sum = bcd_add8(v1, v2, c_in);
        } else {
            sum = v1 + v2 + c_in;
            if (sum > 0xFF) {
                SET_FLAG(C_FLAG);
            } else {
                CLR_FLAG(C_FLAG);
            }
        }
        sum &= 0xFF;
        cpu_state.A.AL = sum;
        change_nzflag(sum, FALSE);
        change_vflag(v1, v2, sum, FALSE);
    }
}

void sbc (cpu_context_t *cpu)
{
    uint32_t sum;
    uint16_t v1;
    uint16_t v2;
    uint8_t borrow_in = 0;

    borrow_in = (GET_FLAG(C_FLAG) == 0);
    if (GET_MSIZE() == 0) {
        // 16 bit add
        v1 = cpu_state.A.C;
        load_temp16();
        v2 = cpu_dynamic_metadata.TEMP;
        if (GET_FLAG(D_FLAG)) {
            sum = bcd_sub16(v1, v2, borrow_in);
        } else {
            sum = v1 - v2 - borrow_in;
            if (sum > 0xFFFF) {
                CLR_FLAG(C_FLAG);   // > 2^16 means borrow occurred
            } else {
                SET_FLAG(C_FLAG);
            }
        }
        sum &= 0xFFFF;
        cpu_state.A.C = sum;
        change_nzflag(sum, TRUE);
        change_vflag(v1, v2, sum, TRUE);
    } else {
        // 8 bits
        v1 = cpu_state.A.AL;
        load_temp8();
        v2 = cpu_dynamic_metadata.TEMP & 0xFF;
        if (GET_FLAG(D_FLAG)) {
            sum = bcd_sub8(v1, v2, borrow_in);
        } else {
            sum = v1 - v2 - borrow_in;
            if (sum > 0xFF) {
                CLR_FLAG(C_FLAG);   // FIXME: definitely wrong
            } else {
                SET_FLAG(C_FLAG);
            }
        }
        sum &= 0xFF;
        cpu_state.A.AL = sum;
        change_nzflag(sum, FALSE);
        change_vflag(v1, v2, sum, FALSE);
    }
}


uint8_t shift_left8 (cpu_context_t *cpu, uint8_t val)
{
    if ((val & 0x80) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val << 1;
    change_nzflag(cpu, val, FALSE);
    return val;
}

uint16_t shift_left16 (cpu_context_t *cpu, uint16_t val)
{
    if ((val & 0x8000) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val << 1;
    change_nzflag(cpu, val, TRUE);
    return val;
}

uint16_t shift_right16 (cpu_context_t *cpu, uint16_t val)
{
    if ((val & 1) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val >> 1;
    change_nzflag(cpu, val, TRUE);
    return val;
}

uint16_t rotate_left16 (cpu_context_t *cpu, uint16_t val)
{
    uint16_t prev;

    prev = GET_FLAG(C_FLAG);

    if ((val & 0x8000) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val << 1;
    val |= (prev & 0x1);
    change_nzflag(cpu, val, TRUE);
    return val;
}

uint16_t rotate_left8 (cpu_context_t *cpu, uint8_t val)
{
    uint8_t prev;

    prev = get_flag(cpu, C_FLAG);

    if ((val & 0x80) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val << 1;
    val |= (prev & 0x1);
    change_nzflag(cpu, val, TRUE);
    return val;
}

uint16_t shift_right8 (cpu_context_t *cpu, uint8_t val)
{
    if ((val & 0x80) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val << 1;
    change_nzflag(cpu, val, FALSE);
    return val;
}

uint16_t rotate_right16 (cpu_context_t *cpu, uint16_t val)
{
    uint16_t prev;

    prev = get_flag(cpu, C_FLAG);

    if ((val & 1) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val >> 1;
    val |= (prev << 15);
    change_nzflag(cpu, val, TRUE);
    return val;
}

uint16_t rotate_right8 (cpu_context_t *cpu, uint8_t val)
{
    uint8_t prev;

    prev = get_flag(cpu, C_FLAG);

    if ((val & 1) == 0) {
        clr_flag(cpu, C_FLAG);
    } else {
        set_flag(cpu, C_FLAG);
    }
    val = val >> 1;
    val |= (prev << 7);
    change_nzflag(cpu, val, FALSE);
    return val;
}

// Unlike ADC, which always uses the accumulator for the destination,
// regardless of the operand source, this instruction has the same
// destination and source.  Thus, if the source is the accumulator,
// so is the destination.  If the source is memory, then the destination
// is the same memory.  Instructions like this are:
// ASL, ROL, ROR, LSR, DEC, INC.
// ASL A, ROL A, ROR A, LSR A, DEC A, INC A operate entirely in A
// and all other variants are read/modify/write to memory

void inc (cpu_context_t *cpu)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;

    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_MSIZE() == 0) {
            op = cpu_state.A.C;
            ++op;
            change_nzflag(op, TRUE);
            cpu_state.A.C = op;
        } else {
            op_l = cpu_state.A.AL;
            ++op_l;
            change_nzflag(op_l, FALSE);
            cpu_state.A.AL = op_l;
        }
    } else {
        // Memory source/destination
        if (GET_MSIZE() == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            ++op;
            cpu_dynamic_metadata.TEMP = op;
            change_nzflag(op, TRUE);
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            ++op_l;
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            change_nzflag(op_l, FALSE);
            store_temp8();  // Put it back into memory
        }

    }
}

void dec (cpu_context_t *cpu)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;

    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_MSIZE() == 0) {
            op = cpu_state.A.C;
            op--;
            change_nzflag(op, TRUE);
            cpu_state.A.C = op;
        } else {
            op_l = cpu_state.A.AL;
            op_l--;
            change_nzflag(op_l, FALSE);
            cpu_state.A.AL = op_l;
        }
    } else {
        // Memory source/destination
        if (GET_MSIZE() == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            op--;
            change_nzflag(op, TRUE);
            cpu_dynamic_metadata.TEMP = op;
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            op_l--;
            change_nzflag(op_l, FALSE);
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            store_temp8();  // Put it back into memory
        }

    }
}


void lsr (cpu_context_t *cpu)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;

    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_MSIZE() == 0) {
            op = cpu_state.A.C;
            op = shift_right16(op);
            cpu_state.A.C = op;
            change_nzflag(op, TRUE);
        } else {
            op_l = cpu_state.A.AL;
            op_l = shift_right8(op_l);
            cpu_state.A.AL = op_l;
            change_nzflag(op_l, FALSE);
        }
    } else {
        // Memory source/destination
        if (GET_MSIZE() == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            op = shift_right16(op);
            change_nzflag(op, TRUE);
            cpu_dynamic_metadata.TEMP = op;
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            op_l = shift_right8(op_l);
            change_nzflag(op_l, FALSE);
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            store_temp8();  // Put it back into memory
        }

    }
}

void asl (cpu_context_t *cpu)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;

    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_MSIZE() == 0) {
            op = cpu_state.A.C;
            op = shift_left16(op);
            change_nzflag(op, TRUE);
            cpu_state.A.C = op;
        } else {
            op_l = cpu_state.A.AL;
            op_l = shift_left8(op_l);
            change_nzflag(op_l, FALSE);
            cpu_state.A.AL = op_l;
        }
    } else {
        // Memory source/destination
        if (GET_MSIZE() == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            op = shift_left16(op);
            cpu_dynamic_metadata.TEMP = op;
            change_nzflag(op, TRUE);
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            op_l = shift_left8(op_l);
            change_nzflag(op_l, FALSE);
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            store_temp8();  // Put it back into memory
        }

    }
}

void rol (cpu_context_t *cpu)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;

    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_MSIZE() == 0) {
            op = cpu_state.A.C;
            op = rotate_left16(op);
            change_nzflag(op, TRUE);
            cpu_state.A.C = op;
        } else {
            op_l = cpu_state.A.AL;
            op_l = rotate_left8(op_l);
            change_nzflag(op_l, FALSE);
            cpu_state.A.AL = op_l;
        }
    } else {
        // Memory source/destination
        if (GET_MSIZE() == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            op = rotate_left16(op);
            cpu_dynamic_metadata.TEMP = op;
            change_nzflag(op, TRUE);
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            op_l = rotate_left8(op_l);
            change_nzflag(op_l, FALSE);
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            store_temp8();  // Put it back into memory
        }

    }
}

void ror (cpu_context_t *cpu)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;

    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_MSIZE() == 0) {
            op = cpu_state.A.C;
            op = rotate_left16(op);
            cpu_state.A.C = op;
            change_nzflag(op, TRUE);
        } else {
            op_l = cpu_state.A.AL;
            op_l = rotate_left8(op_l);
            change_nzflag(op_l, FALSE);
            cpu_state.A.AL = op_l;
        }
    } else {
        // Memory source/destination
        if (GET_MSIZE() == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            op = rotate_left16(op);
            cpu_dynamic_metadata.TEMP = op;
            change_nzflag(op, TRUE);
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            op_l = rotate_left8(op_l);
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            change_nzflag(op_l, FALSE);
            store_temp8();  // Put it back into memory
        }

    }
}
#endif

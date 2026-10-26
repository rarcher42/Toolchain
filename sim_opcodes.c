#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sim6502.h"
#include "sim_opcodes.h"
#include "sim_computer.h"

static inline uint8_t get_A (void)
{
    return cpu.reg.A;
}

static inline uint8_t get_X (void)
{
    return cpu.reg.X;
}

static inline uint8_t get_Y (void)
{
    return cpu.reg.Y;
}


static inline uint8_t get_raw_S (void)
{
    return cpu.reg.S;
}


static inline uint8_t get_P (void)
{
    return cpu.reg.P;
}

static inline uint16_t get_PC (void)
{
    return cpu.reg.PC;
}

void change_zflag (uint8_t val)
{
    if (val == 0) {
        set_flag(Z_FLAG);
    } else {
        clr_flag(Z_FLAG);
    }
}

// Many instructions set N and Z flag by value.
// N is set the MSB of result,
// Z is set by whether the result is zero
void change_nzflag (uint8_t val)
{
    change_zflag(val);

    if ((val & 0x80) == 0) {
        clr_flag(N_FLAG);
    } else {
        set_flag(N_FLAG);
    }
}

static void push8 (uint8_t val)
{
    uint16_t sp;

    sp = (cpu.reg.S | 0x100) & 0x1FF;
    cpu_write(sp, val);
    cpu.reg.S = (--sp & 0xFF);
}

#if 0
static void push16 (uint16_t val)
{
    push8((val >> 8) & 0xFF);
    push8(val & 0xFF);
}
#endif

uint8_t pop8 (void)
{
    uint16_t sp;
    uint8_t val;

    sp = (cpu.reg.S + 1) | 0x100;
    cpu.reg.S = (sp & 0xFF);
    val = cpu_read(sp);
    return val;
}

uint16_t pop16 (void)
{
    uint16_t val;
    uint8_t lsb;
    lsb = pop8();
    val = (pop8() << 8) | lsb;
    return val;
}

void nop (void)
{
    // Add cycle count
}

// Fun with flags
void clc (void)
{
    clr_flag(C_FLAG);
}

void sec (void)
{
    set_flag(C_FLAG);
}

void cld (void)
{
    clr_flag(D_FLAG);
}

void sed (void)
{
    set_flag(D_FLAG);
}

void sei (void)
{
    set_flag(I_FLAG);
}

void cli (void)
{
    clr_flag(I_FLAG);
}

void clv (void)
{
    clr_flag(V_FLAG);
}

/* Note:  there is no set overflow instruction */

void sep (void)
{
    printf("\nNot implemented!");
    exit(0);
}

void rep (void)
{
    printf("\nNot implemented!");
    exit(0);
}


void dex (void)
{
    uint8_t temp;

    temp = cpu.reg.X - 1;
    cpu.reg.X = temp;
    change_nzflag(temp);
}

void dey (void)
{
    uint8_t temp;

    temp = cpu.reg.Y - 1;
    cpu.reg.Y = temp;
    change_nzflag(temp);
}


void inx (void)
{
    uint8_t temp;

    temp = cpu.reg.X + 1;
    cpu.reg.X = temp;
    change_nzflag(temp);
}

void iny (void)
{
    uint8_t temp;

    temp = cpu.reg.Y + 1;
    cpu.reg.Y = temp;
    change_nzflag(temp);
}

void phb (void)
{
    // push8(cpu_state.DBR);
}

void phk (void)
{
    // push8(cpu_state.PBR);
}

void phd (void)
{
    // push16(cpu_state.DPR);
}

void php (void)
{
    push8(cpu.reg.P);
}

void pha (void)
{
    push8(cpu.reg.A);
}

void phx (void)
{
}

void phy (void)
{
}

void pla (void)
{
    uint8_t vb;

    vb = pop8();
    cpu.reg.A = vb;
    change_nzflag(vb);
}

void plb (void)
{
}

void pld (void)
{
}

void plp (void)
{
    uint8_t vb;

    vb = pop8();
    cpu.reg.P = vb;
    // Don't set NV flags - they remain exactly as pulled from stack!
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

void tax (void)
{
    uint8_t temp;

    temp = cpu.reg.A;
    cpu.reg.X = temp;
    change_nzflag(temp);
}

void tay (void)
{
    uint8_t temp;

    temp = cpu.reg.A;
    cpu.reg.Y = temp;
    change_nzflag(temp);
}

void txa (void)
{
    uint8_t temp;

    temp = cpu.reg.X;
    cpu.reg.A = temp;
    change_nzflag(temp);
}

void tya (void)
{
    uint8_t temp;

    temp = cpu.reg.Y;
    cpu.reg.A = temp;
    change_nzflag(temp);
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
    uint8_t temp;

    temp = cpu.reg.S;
    cpu.reg.X = temp;
    change_nzflag(temp);
}

void txs (void)
{
    cpu.reg.S = cpu.reg.X;
    // No flag changes!
}


void txy (void)
{
}

void tyx (void)
{
}


#if 0
void bcs (void)
{
    if (get_flag(C_FLAG)) {
        put_PC(get_EA());
    }
}

void bcc (void)
{
    if (get_flag(C_FLAG) == 0) {
        put_PC(get_EA());
    }
}

void bne (void)
{
    if (get_flag(Z_FLAG) == 0) {
        put_PC(get_EA());
    }
}

void beq (void)
{
    if (get_flag(Z_FLAG)) {
        put_PC(get_EA());
    }
}

void bmi (void)
{
    if (get_flag(N_FLAG)) {
        put_PC(get_EA());
    }
}

void bpl (void)
{
    if (get_flag(N_FLAG) == 0) {
        put_PC(get_EA());
    }
}

void bvs (void)
{
    if (get_flag(V_FLAG)) {
        put_PC(get_EA());
    }
}

void bvc (void)
{
    if (get_flag(V_FLAG) == 0) {
        put_PC(get_EA());
    }
}


void stp (void)
{
    printf("Unimplemented!");
    exit(0);
}

void brak (void)
{
    uint8_t lsb, msb;

    printf("\nBRK - STOPPING!\n");
    if (IS_EMU(
        push16(cpu_state.SP + 2);
        push8(cpu_state.flags | B_FLAG); // 65x02 doesn't set B flag, ONLY on stack copy!
        SET_FLAG(I_FLAG);
        lsb = cpu_read(0xFFFE);
        msb = cpu_read(0xFFFF);
        cpu_state.PC = (msb << 8) | lsb;
    }
}

void unimp (void)
{
    printf("\nUnimplemented op-code %02X: aborting\n", get_ir_opcode());
    cpu_dynamic_metadata.running = FALSE;
}

void xce (void)
{
}

// Branches and Jumps
void bra (void)
{
    cpu_state.PC = get_EA();
}

void brl (void)
{
    cpu_state.PC = get_EA();
}



void jmp (void)
{
    cpu_state.PC = get_EA() - 3;
}

void jsr (void)
{
    uint16_t target;
    uint16_t ra;

    target = get_EA() - 3;  //
    ra = cpu_state.PC + 1; // 6502 family writes RA-1 to stack
    push16(ra);
    cpu_state.PC = target;
}

void jsl (void)
{
}

void rts (void)
{
    uint16_t ra;

    ra = pop16() + 1;   // Increment ra to point to next instruction
    cpu_state.PC = ra;
}

void rti (void)
{
    uint16_t ra;
    uint8_t flags;

    ra = pop16() + 1;   // Increment ra to point to next instruction
    flags = pop8();
    // FIXME: must handle PBR from context.  Clean up CPU context first
    cpu_state.flags = flags;
    cpu_state.PC = ra;
}

void rtl (void)
{
}

void jml (void)
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

void cpx (void)
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

void cpy (void)
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

void cmp (void)
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

void xba (void)
{
    uint8_t temp;

    temp = cpu_state.A.B;
    cpu_state.A.B = cpu_state.A.AL;
    cpu_state.A.AL = temp;
    change_nzflag(temp, FALSE);
}

void lda (void)
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

void ldx (void)
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

void ldy (void)
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

void tsb (void)
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

void trb (void)
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

void sta (void)
{
    if (GET_MSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.A.C;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.A.AL & 0xFF);
        store_temp8();
    }
}

void stz (void)
{
    if (GET_MSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = (uint16_t) 0;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (uint8_t) 0;
        store_temp8();
    }
}

void stx (void)
{
    if (GET_XSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.X;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.X & 0xFF);
        store_temp8();
    }
}

void sty (void)
{
    if (GET_XSIZE() == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.Y;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.Y & 0xFF);
        store_temp8();
    }
}


void anl (void)
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

void bit (void)
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


void eor (void)
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

void ora (void)
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

void adc (void)
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

    // 8 bits
    v1 = cpu.A;
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

void sbc (void)
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

void inc (void)
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

void dec (void)
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


void lsr (void)
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

void asl (void)
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

void rol (void)
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

void ror (void)
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

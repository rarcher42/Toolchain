#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "disasm.h"
#include "sim.h"
#include "calc_ea.h"
#include "opcodes.h"

// Many instructions set N and Z flag by value.
// N is set the MSB of result, 
// Z is set by whether the result is zero
void change_nzflag(uint16_t val, BOOL sixteen)
{
    uint16_t mask;
    
    if (sixteen) {
        mask = 0x8000;
    } else {
        mask = 0x80;
    }
    
    if (val == 0) {
        SET_FLAG(Z_FLAG);
    } else {
        CLR_FLAG(Z_FLAG);
    }
    
    if ((val & mask) == 0) {
        CLR_FLAG(N_FLAG);
    } else {
        SET_FLAG(N_FLAG);
    } 
}

static void push8 (uint8_t val)
{
    uint16_t sp;
    
    sp = cpu_state.SP;
    cpu_write(sp, val);
    cpu_state.SP = sp - 1;  
}

static void push16 (uint16_t val)
{
    push8((val >> 8) & 0xFF);
    push8(val & 0xFF);
}

uint8_t pop8 (void)
{
    uint16_t sp;
    uint8_t val;
    
    sp = cpu_state.SP + 1;
    cpu_state.SP = sp;
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
    
}

void clc (void)
{
    CLR_FLAG(C_FLAG);
}

void sec (void)
{
    SET_FLAG(C_FLAG);
}

void cld (void)
{
    CLR_FLAG(D_FLAG);
}

void sed (void)
{
    SET_FLAG(D_FLAG);
}

void sei (void)
{
    SET_FLAG(I_FLAG);
}

void cli (void)
{
    CLR_FLAG(I_FLAG);
}

void clv (void)
{
    CLR_FLAG(V_FLAG);
}

void sep (void)
{
    uint8_t val;
    
    val = get_ir_indexed(1);
    cpu_state.flags |= val;
}

void rep (void)
{
    uint8_t val;
    
    val = get_ir_indexed(1);
    cpu_state.flags &= ~val;
}

void inx (void)
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        temp = cpu_state.X + 1;
        change_nzflag(temp, TRUE);
        cpu_state.X = temp;
    } else {
        temp = (cpu_state.X + 1) & 0xFF;
        change_nzflag(temp, FALSE);
        cpu_state.X = temp & 0xFF;
    }
}

void iny (void)
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        temp = cpu_state.Y + 1;
        change_nzflag(temp, TRUE);
        cpu_state.Y = temp;
    } else {
        temp = (cpu_state.Y + 1) & 0xFF;
        change_nzflag(temp, FALSE);
        cpu_state.Y = temp & 0xFF;
    }
}

void dex (void)
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        temp = cpu_state.X - 1;
        change_nzflag(temp, TRUE);
        cpu_state.X = temp;
    } else {
        temp = (cpu_state.X - 1) & 0xFF;
        change_nzflag(temp, FALSE);
        cpu_state.X = temp & 0xFF;
    }
}

void dey (void)
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        temp = cpu_state.Y - 1;
        change_nzflag(temp, TRUE);
        cpu_state.Y = temp;
    } else {
        temp = (cpu_state.Y - 1) & 0xFF;
        change_nzflag(temp, FALSE);
        cpu_state.Y = temp & 0xFF;
    }
}

void stp (void)
{
    printf("STP - STOPPING!\n");
    cpu_dynamic_metadata.running = FALSE;
}

void brk (void)
{
	uint8_t lsb, msb;
	
    printf("BRK - STOPPING!\n");
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

void unimp (void)
{
    printf("\nUnimplemented op-code %02X: aborting\n", get_ir_opcode());
    cpu_dynamic_metadata.running = FALSE;
}

void xce (void)
{
    BOOL carry;
    
    carry = IS_EMU();
    if (GET_FLAG(C_FLAG)) {
        SET_EMU(TRUE);
    } else {
        SET_EMU(FALSE);
    }
    if (carry) {
        SET_FLAG(C_FLAG);
    } else {
        CLR_FLAG(C_FLAG);
    }
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

void bcs (void)
{
    if (GET_FLAG(C_FLAG)) {
        cpu_state.PC = get_EA();
    }
}

void bcc (void)
{
    if (GET_FLAG(C_FLAG) == 0) {
        cpu_state.PC = get_EA();
    }
}

void bne (void)
{
    if (GET_FLAG(Z_FLAG) == 0) {
        cpu_state.PC = get_EA();
    }
}

void beq (void)
{
    if (GET_FLAG(Z_FLAG)) {
        cpu_state.PC = get_EA();
    }
}

void bmi (void)
{
    if (GET_FLAG(N_FLAG)) {
        cpu_state.PC = get_EA();
    }
}

void bpl (void)
{
    if (GET_FLAG(N_FLAG) == 0) {
        cpu_state.PC = get_EA();
    }
}

void bvs (void)
{
    if (GET_FLAG(V_FLAG)) {
        cpu_state.PC = get_EA();
    }
}

void bvc (void)
{
    if (GET_FLAG(V_FLAG) == 0) {
        cpu_state.PC = get_EA();
    }   
}

void jmp (void)
{
    cpu_state.PC = get_EA();
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
    uint16_t ra;

    push8(cpu_state.PBR);
    ra =  cpu_state.PC + 2; // 6502 family writes RA - 1 to stack
    push16(ra);
    cpu_state.PC = get_EA() - 4;
    cpu_state.PBR = get_ir_indexed(3);
}

void rts (void)
{
    uint16_t ra;
    
    ra = pop16() + 1;   // Increment ra to point to next instruction
    cpu_state.PC = ra;
}

void rtl (void)
{
    uint16_t ra;
    uint8_t bank;
    
    ra = pop16() + 1;   // Increment ra to point to next instruction
    bank = pop8();
    
    cpu_state.PC = ra;
    cpu_state.PBR = bank;
}

void jml (void)
{
    cpu_state.PC = get_EA();
    cpu_state.PBR = get_ir_indexed(3);  // Get PBR from instruction stream
}

void tax (void)
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        // 16 bit transfer to X
        if (GET_FLAG(M_FLAG) == 0) {
            temp = cpu_state.A.C;
            cpu_state.X = temp;
            change_nzflag(temp, TRUE);
        } else {
            temp = ((cpu_state.A.B & 0xFF) << 8) | (cpu_state.A.AL);
            cpu_state.X = temp;
            change_nzflag(temp, TRUE);
        }
    } else {
        temp = (cpu_state.A.AL) & 0xFF;
        cpu_state.X = temp;
        change_nzflag(temp, FALSE);
        // 8 bit transfer to Y from 8 bit A
    }
}

void tay (void)
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        // 16 bit transfer to X
        if (GET_FLAG(M_FLAG) == 0) {
            temp = cpu_state.A.C;
            cpu_state.Y = temp;
            change_nzflag(temp, TRUE);
        } else {
            temp = ((cpu_state.A.B & 0xFF) << 8) | (cpu_state.A.AL);
            cpu_state.Y = temp;
            change_nzflag(temp, TRUE);
        }
    } else {
        temp = (cpu_state.A.AL) & 0xFF;
        cpu_state.Y = temp;
        change_nzflag(temp, FALSE);
        // 8 bit transfer to Y from 8 bit A
    }
}

void tcd (void)
{
    uint16_t temp;
    
    temp = cpu_state.A.C;
    cpu_state.DPR = temp;
    change_nzflag(temp, TRUE);
}

void tcs (void)
{   
    if (IS_EMU()) {
        cpu_state.SP = 0x100 | (cpu_state.A.AL & 0xFF);
        // No flags are affected!
    } else {
        // 16 bits regardless of M flag setting 
        cpu_state.SP = cpu_state.A.C;
        // No flags are affected
    }
}

void tdc (void)
{
    uint16_t temp;
    temp = cpu_state.DPR;
    cpu_state.A.C = temp;
    change_nzflag(temp, TRUE);
}

void tsc (void)
{
    uint16_t temp;
    
    temp = cpu_state.SP;
    cpu_state.A.C = temp;
    change_nzflag(temp, TRUE);
}

void tsx (void) 
{
    uint16_t temp;
    
    if (GET_FLAG(X_FLAG) == 0) {
        // 16 bit transfer
        temp = cpu_state.SP;
        cpu_state.X = temp;
        change_nzflag(temp, TRUE);
    } else {
        // 8 bit transfer
        if (IS_EMU()) {
            temp = (cpu_state.SP & 0xFF) | 0x100;
        } else {
            temp = cpu_state.SP;
        }
        cpu_state.X = temp;
        change_nzflag(temp & 0xFF, FALSE);
    }
}

void txs (void)
{
    if (GET_FLAG(X_FLAG)) {
		// 8 bit mode, Native, put 00xx into SP
		// reference pg. 518 Programming the 65816..
		// by Western Design Center TXS page
		cpu_state.SP = cpu_state.X & 0xFF;
		if (IS_EMU()) {
			cpu_state.SP |= 0x100;
		}
    } else {
        cpu_state.SP = cpu_state.X;
    }
}


void txy (void)
{
    uint16_t val;
    uint16_t sav;
    
    val = cpu_state.X;
    sav = (cpu_state.Y) & 0xFF00;
    if (GET_FLAG(X_FLAG) == 0) {
        // 16 bits
        val = cpu_state.X;
        cpu_state.Y = val;
        change_nzflag(val, TRUE);
    } else {
        // 8 bits
        val = cpu_state.X | sav;
        cpu_state.Y = val;
        change_nzflag(val & 0xFF, FALSE);
    }
}

void tyx (void)
{
    uint16_t val;
    uint16_t sav;
    
    val = cpu_state.Y;
    sav = (cpu_state.X) & 0xFF00;
    if (GET_FLAG(X_FLAG) == 0) {
        // 16 bits
        val = cpu_state.Y;
        cpu_state.X = val;
        change_nzflag(val, TRUE);
    } else {
        // 8 bits
        val = cpu_state.Y | sav;
        cpu_state.X = val;
        change_nzflag(val & 0xFF, FALSE);
    }
}

void txa (void)
{
    uint temp;
    
    if (GET_FLAG(M_FLAG) == 0) {
        // 16 bits
        if (GET_FLAG(X_FLAG)) {
            temp = cpu_state.X & 0xFF;
        } else {
            temp = cpu_state.X;
        }
        cpu_state.A.C = temp;
        change_nzflag(temp, TRUE);
    } else {
        temp = cpu_state.X & 0xFF;
        cpu_state.A.AL = temp;
        change_nzflag(temp & 0xFF, FALSE);
        // 8 bits 
    }
}

void tya (void)
{
    uint temp;
    
    if (GET_FLAG(M_FLAG) == 0) {
        // 16 bits
        if (GET_FLAG(X_FLAG)) {
            temp = cpu_state.Y & 0xFF;
        } else {
            temp = cpu_state.Y;
        }
        cpu_state.A.C = temp;
        change_nzflag(temp, TRUE);
    } else {
        temp = cpu_state.Y & 0xFF;
        cpu_state.A.AL = temp;
        change_nzflag(temp & 0xFF, FALSE);
        // 8 bits 
    }
}



void phb (void)
{
    push8(cpu_state.DBR);
}

void phk (void)
{
    push8(cpu_state.PBR);
}

void phd (void)
{
    push16(cpu_state.DPR);
}

void php (void)
{
    push8(cpu_state.flags);
}

void pha (void)
{
    if (GET_FLAG(M_FLAG)) {
        push8(cpu_state.A.AL);
    } else {
        push16(cpu_state.A.C);
    }
}

void phx (void)
{
    if (GET_FLAG(X_FLAG)) {
        push8(cpu_state.X & 0xFF);
    } else {
        push16(cpu_state.X);
    }
}

void phy (void)
{
    if (GET_FLAG(X_FLAG)) {
        push8(cpu_state.Y & 0xFF);
    } else {
        push16(cpu_state.Y);
    }
}

void pla (void)
{
    uint16_t v;
    uint8_t vb;
    
    if (GET_FLAG(M_FLAG)) {
        // 8 bits
        vb = pop8();
        cpu_state.A.AL = vb;
        change_nzflag(vb, FALSE);
    } else {
        // 16 bits
        v = pop16();
        cpu_state.A.C = v;
        change_nzflag(v, TRUE);
    } 
}

void plb (void)
{
    uint8_t vb;
    
    vb = pop8();
    cpu_state.DBR = vb;
    change_nzflag(vb, FALSE);
}

void pld (void)
{
    uint16_t v;
    
    v = pop16();
    cpu_state.DPR = v;
    change_nzflag(v, TRUE);
}

void plp (void)
{
    uint8_t vb;
    
    vb = pop8();
    cpu_state.flags = vb;
    change_nzflag(vb, FALSE);
}

void plx(void)
{
    uint16_t v;
    
    v = pop16();
    cpu_state.X = v;
    change_nzflag(v, TRUE);
}

void ply (void)
{
    uint16_t v;
    
    v = pop16();
    cpu_state.Y = v;
    change_nzflag(v, TRUE);
}

void plk (void)
{
    uint8_t vb;
    
    vb = pop8();
    cpu_state.PBR = vb;
    change_nzflag(vb, FALSE);
}


void set_compare_borrow (uint32_t diff, BOOL sixteen)
{
    if (sixteen) {
        if (diff > 0xFFFF) {
            CLR_FLAG(C_FLAG);   // A - M caused a borrow!
        } else {
            SET_FLAG(C_FLAG);   // A - M resulted in no borrow
        }
    } else {
        if (diff > 0xFF) {
            CLR_FLAG(C_FLAG);   // A - M caused borrow!
        } else {
            SET_FLAG(C_FLAG);   // A - M no borrow
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
    
    if (GET_FLAG(X_FLAG) == 0) {
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

    
    if (GET_FLAG(X_FLAG) == 0) {
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
    
    if (GET_FLAG(X_FLAG) == 0) {
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

void lda (void)
{
    if (GET_FLAG(M_FLAG) == 0) {
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
    if (GET_FLAG(X_FLAG) == 0) {
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
    if (GET_FLAG(X_FLAG) == 0) {
        load_temp16();
        cpu_state.Y = cpu_dynamic_metadata.TEMP;
        change_nzflag(cpu_state.Y, TRUE);
    } else {
        load_temp8();
        cpu_state.Y = (cpu_dynamic_metadata.TEMP & 0xFF);
        change_nzflag(cpu_state.Y, FALSE);
    }
}


void sta (void)
{   
    if (GET_FLAG(M_FLAG) == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.A.C;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.A.AL & 0xFF);
        store_temp8();
    }
}

void stx (void)
{   
    if (GET_FLAG(X_FLAG) == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.X;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.X & 0xFF);
        store_temp8();
    }
}

void sty (void)
{   
    if (GET_FLAG(X_FLAG) == 0) {
        cpu_dynamic_metadata.TEMP = cpu_state.Y;
        store_temp16();
    } else {
        cpu_dynamic_metadata.TEMP = (cpu_state.Y & 0xFF);
        store_temp8();
    }
}

void sbc (void)
{
    uint32_t sum;
    uint16_t v1;
    uint16_t v2;
    
    // FIXME: Handle D-flag
    if (GET_FLAG(D_FLAG)) {
        printf("\n*** ERROR: Unhandled decimal flag! ***\n");
        exit(0);
    }
    if (GET_FLAG(M_FLAG) == 0) {
        // 16 bit add
        v1 = cpu_state.A.C;
        load_temp16();
        v2 = cpu_dynamic_metadata.TEMP;
        sum = v1 - v2;
        if (GET_FLAG(C_FLAG) == 0) {
            sum = sum - 1;
        }
        if (sum > 0xFFFF) {
            CLR_FLAG(C_FLAG);   // > 2^16 means borrow occurred
        } else {
            SET_FLAG(C_FLAG);
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
        // FIXME: Handle
        sum = v1 - v2;
        if (GET_FLAG(C_FLAG) == 0) {
            sum = sum - 1;
        }
        if (sum > 0xFF) {
            CLR_FLAG(C_FLAG);   // FIXME: definitely wrong
        } else {
            SET_FLAG(C_FLAG);
        }
        sum &= 0xFF;
        cpu_state.A.AL = sum;
        change_nzflag(sum, FALSE);
        change_vflag(v1, v2, sum, FALSE);
    }
}

void andop (void)
{
    uint16_t v1;
    uint16_t v2;
    uint16_t res;
    uint8_t v1_l;
    uint8_t v2_l;
    uint8_t res_l;
    
    
    if (GET_FLAG(M_FLAG) == 0) {
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

void eor (void)
{
    uint16_t v1;
    uint16_t v2;
    uint16_t res;
    uint8_t v1_l;
    uint8_t v2_l;
    uint8_t res_l;
    
    
    if (GET_FLAG(M_FLAG) == 0) {
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
    
    
    if (GET_FLAG(M_FLAG) == 0) {
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

void adc (void)
{
    uint32_t sum;
    uint16_t v1;
    uint16_t v2;
    
    // FIXME: Handle D-flag
    if (GET_FLAG(D_FLAG)) {
        printf("\n*** ERROR: Unhandled decimal flag! ***\n");
        exit(0);
    }
    if (GET_FLAG(M_FLAG) == 0) {
        // 16 bit add
        v1 = cpu_state.A.C;
        load_temp16();
        v2 = cpu_dynamic_metadata.TEMP;
        sum = v1 + v2;
        if (GET_FLAG(C_FLAG)) {
            sum = sum + 1;
        }
        cpu_state.A.C = sum;
        if (sum > 0xFFFF) {
            SET_FLAG(C_FLAG);
        } else {
            CLR_FLAG(C_FLAG);
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
        sum = v1 + v2;
        if (GET_FLAG(C_FLAG)) {
            sum = sum + 1;
        }
        if (sum > 0xFF) {
            SET_FLAG(C_FLAG);
        } else {
            CLR_FLAG(C_FLAG);
        }
        sum &= 0xFF;
        cpu_state.A.AL = sum;
        change_nzflag(sum, FALSE);
        change_vflag(v1, v2, sum, FALSE);
    }
}



uint8_t shift_left8 (uint8_t val)
{
    if ((val & 0x80) == 0) {
        CLR_FLAG(C_FLAG);
    } else {
        SET_FLAG(C_FLAG);
    }
    val = val << 1;
    change_nzflag(val, FALSE);
    return val;
}

uint16_t shift_left16 (uint16_t val)
{
    if ((val & 0x8000) == 0) {
        CLR_FLAG(C_FLAG);
    } else {
        SET_FLAG(C_FLAG);
    }
    val = val << 1;
    change_nzflag(val, TRUE);
    return val; 
}


uint16_t shift_right16 (uint16_t val)
{
    if ((val & 1) == 0) {
        CLR_FLAG(C_FLAG);
    } else {
        SET_FLAG(C_FLAG);
    }
    val = val >> 1;
    change_nzflag(val, TRUE);
    return val; 
}

uint16_t shift_right8 (uint8_t val)
{
    if ((val & 1) == 0) {
        CLR_FLAG(C_FLAG);
    } else {
        SET_FLAG(C_FLAG);
    }
    val = val >> 1;
    change_nzflag(val, FALSE);
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
void lsr (void)
{
    address_mode_t address_mode;
    uint16_t op;
    uint8_t op_l;
    
    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        // Accumulator source/destination
        if (GET_FLAG(M_FLAG) == 0) {
            op = cpu_state.A.C;
            op = shift_right16(op);
            cpu_state.A.C = op;
        } else {
            op_l = cpu_state.A.AL;
            op_l = shift_right8(op_l);
            cpu_state.A.AL = op_l;
        }
    } else {
        // Memory source/destination
        if (GET_FLAG(M_FLAG) == 0) {
            load_temp16();
            op = cpu_dynamic_metadata.TEMP;
            op = shift_right16(op);
            cpu_dynamic_metadata.TEMP = op;
            store_temp16();
        } else {
            load_temp8();
            op_l = (cpu_dynamic_metadata.TEMP & 0xFF);
            op_l = shift_right8(op_l);
            cpu_dynamic_metadata.TEMP = op_l & 0xFF;
            store_temp8();  // Put it back into memory
        }
        
    }
    
}

void asl (void)
{
    uint16_t src_op;
    BOOL dest_is_accumulator;
    address_mode_t address_mode;
    
    address_mode = get_ir_addr_mode();
    if (address_mode == OP_A) {
        dest_is_accumulator = TRUE;
    } else {
        dest_is_accumulator = FALSE;
    }
    
    if (GET_FLAG(M_FLAG) == 0) {
        // 16 bit
        if (dest_is_accumulator) {
            src_op = cpu_state.A.C;
        } else {
            load_temp16();
            src_op = cpu_dynamic_metadata.TEMP;
        }
        // Left shift - MSB goes to carry flag
        if (src_op & 0x8000) {
            SET_FLAG(C_FLAG);
        } else {
            CLR_FLAG(C_FLAG);
        }
        src_op = (src_op << 1) & 0xFFFF;
        if (dest_is_accumulator) {
            cpu_state.A.C = src_op;
        } else {
            cpu_dynamic_metadata.TEMP = src_op;
            store_temp16();
        }
        change_nzflag(src_op, TRUE);
    }  else {
        // 8 bit
               if (dest_is_accumulator) {
            src_op = cpu_state.A.C;
        } else {
            load_temp16();
            src_op = cpu_dynamic_metadata.TEMP;
        }
        // Left shift - MSB goes to carry flag
        if (src_op & 0x8000) {
            SET_FLAG(C_FLAG);
        } else {
            CLR_FLAG(C_FLAG);
        }
        src_op = (src_op << 1) & 0xFFFF;
        if (dest_is_accumulator) {
            cpu_state.A.C = src_op;
        } else {
            cpu_dynamic_metadata.TEMP = src_op;
            store_temp16();
        }
        change_nzflag(src_op, TRUE);
    } 
}


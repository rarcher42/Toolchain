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


void lda (void)
{
    if (GET_FLAG(M_FLAG) == 0) {
        load_temp16();
        cpu_state.A.C = cpu_dynamic_metadata.TEMP;
    } else {
        load_temp8();
        cpu_state.A.AL = (cpu_dynamic_metadata.TEMP & 0xFF);
    }
    
}

void ldx (void)
{
    if (GET_FLAG(X_FLAG) == 0) {
        load_temp16();
        cpu_state.X = cpu_dynamic_metadata.TEMP;
    } else {
        load_temp8();
        cpu_state.X = (cpu_dynamic_metadata.TEMP & 0xFF);
    }
}

void ldy (void)
{
    if (GET_FLAG(X_FLAG) == 0) {
        load_temp16();
        cpu_state.Y = cpu_dynamic_metadata.TEMP;
    } else {
        load_temp8();
        cpu_state.Y = (cpu_dynamic_metadata.TEMP & 0xFF);
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
        if (sum > 0xFFFF) {
            SET_FLAG(C_FLAG);
        } else {
            CLR_FLAG(C_FLAG);
        }
        sum &= 0xFFFF;
        change_vflag(v1, v2, sum, TRUE); 
    } else {
        // 8 bits
        v1 = cpu_state.A.AL;
        load_temp8();
        v2 = cpu_dynamic_metadata.TEMP & 0xFF;
        // FIXME: Handle 
        cpu_state.A.AL = (cpu_dynamic_metadata.TEMP & 0xFF);
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
        change_vflag(v1, v2, sum, FALSE);
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
            SET_FLAG(C_FLAG);	// FIXME: probably wrong
        } else {
            CLR_FLAG(C_FLAG);
        }
        sum &= 0xFFFF;
        change_vflag(v1, v2, sum, TRUE); 
    } else {
        // 8 bits
        v1 = cpu_state.A.AL;
        load_temp8();
        v2 = cpu_dynamic_metadata.TEMP & 0xFF;
        // FIXME: Handle 
        cpu_state.A.AL = (cpu_dynamic_metadata.TEMP & 0xFF);
        sum = v1 - v2;
        if (GET_FLAG(C_FLAG) == 0) {
            sum = sum - 1;
        }
        if (sum > 0xFF) {
            SET_FLAG(C_FLAG);	// FIXME: definitely wrong
        } else {
            CLR_FLAG(C_FLAG);
        }
        sum &= 0xFF;
        change_vflag(v1, v2, sum, FALSE);
    }
}



void stp (void)
{
    printf("STP - STOPPING!\n");
    cpu_dynamic_metadata.running = FALSE;
}

void brk (void)
{
    printf("BRK - STOPPING!\n");
    cpu_dynamic_metadata.running = FALSE;
}

void unimp (void)
{
    printf("\nUnimplemented op-code %02X: aborting\n", get_ir_opcode());
    cpu_dynamic_metadata.running = FALSE;
}

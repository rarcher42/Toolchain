#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "disasm.h"
#include "sim.h"


static inline uint32_t calc_noEA(void)
{
    // Since addresses are at most 24 bits, returning -1 
    // can't be confused with a valid effective address
    return 0xFFFFFFFF;  // > 24 bits means no EA applies
}

uint32_t calc_abs(void)
{
    uint32_t ea;
    uint16_t base;
    uint8_t lsb, msb, bank;
    
    lsb = get_ir_indexed(1);   // LSB 
    msb = get_ir_indexed(2);   // MSB
    base = (msb << 8) | lsb; 
        
    if (is_65816()) {
        if (IS_EMU()) {
            bank = 0x00;    // ?? FIXME: confirm DBR doesn't matter
            ea = base;
        } else {
            bank = get_dbr();
            ea = (bank << 16) | base;
        }
    } else {
        // Legacy modes:  No DBR
        ea = base;
    }
    printf("OP_ABS($%08X)", ea);
    return ea;
}

uint32_t calc_abs_l(void)
{
    uint32_t ea;
    uint8_t lsb, msb, bank;
    
    lsb = get_ir_indexed(1);   // LSB 
    msb = get_ir_indexed(2);   // MSB
    bank = get_ir_indexed(3);   // Bank address
        
    if (is_65816()) {
        ea = (bank << 16) | (msb << 8) | lsb;
        
    } else {
        // Legacy modes:  No DBR
        printf("*** ERROR - 24 bit operand in legacy mode! ***");
        ea = 0xFFFFFFFF;
    }
    printf("OP_ABS_L($%08X)", ea);
    return ea;
}

uint32_t calc_abs_x(void)
{
    uint32_t ea;
    uint16_t base;
    uint8_t lsb, msb, bank;
    
    lsb = get_ir_indexed(1);   // LSB 
    msb = get_ir_indexed(2);   // MSB 
    base = (msb << 8) | lsb;
    
    if (is_65816()) {
        if (IS_EMU()) {
            /* FIXME: confirm assumption: bank not used */
            /* FIXME: confirm assumption: bank overflow wraps */
            ea = (base + (cpu_state.X & 0xFF)) & 0xFFFF;
        } else {
            bank = get_dbr();
            ea = (bank << 16) | base;   
            if (GET_FLAG(X_FLAG)) {
                ea = ea + (cpu_state.X & 0xFF);
                /* FIXME: confirm no bank wrap */
            } else {
                ea = ea + (cpu_state.X);
                /* FIXME: confirm no bank wrap */
            }
        }
    } else {
        // Legacy modes:  No DBR, no bank wrap
        ea = (base + (cpu_state.X & 0xFF)) & 0xFFFF;
    }
    printf("OP_ABS_X($%08X)", ea); 
    return ea;      
}

uint32_t calc_abs_y(void)
{
    uint32_t ea;
    uint16_t base;
    uint8_t lsb, msb, bank;
    
    lsb = get_ir_indexed(1);   // LSB 
    msb = get_ir_indexed(2);   // MSB 
    base = (msb << 8) | lsb;
    
    if (is_65816()) {
        if (IS_EMU()) {
            /* FIXME: confirm assumption: bank not used */
            /* FIXME: confirm assumption: bank overflow wraps */
            ea = (base + (cpu_state.Y & 0xFF)) & 0xFFFF;
        } else {
            bank = get_dbr();
            ea = (bank << 16) | base;   
            if (GET_FLAG(X_FLAG)) {
                ea = ea + (cpu_state.Y & 0xFF);
                /* FIXME: confirm no bank wrap */
            } else {
                ea = ea + (cpu_state.Y);
                /* FIXME: confirm no bank wrap */
            }
        }
    } else {
        // Legacy modes:  No DBR, no bank wrap
        ea = (base + (cpu_state.Y & 0xFF)) & 0xFFFF;
    }
    printf("OP_ABS_Y($%08X)", ea); 
    return ea;      
}

uint32_t calc_abs_x_l(void)
{
    uint32_t ea;
    uint32_t base;
    uint8_t lsb, msb, bank;
    
    lsb = get_ir_indexed(1);    // LSB 
    msb = get_ir_indexed(2);    // MSB 
    bank = get_ir_indexed(3);   // Bank
    base = (bank << 16) | (msb << 8) | lsb;
    if (is_65816()) {   
        if (GET_FLAG(X_FLAG)) {
            ea = base + (cpu_state.X & 0xFF);
            /* FIXME: confirm no bank wrap */
        } else {
            ea = base + (cpu_state.X);
            /* FIXME: confirm no bank wrap */
        }
    } else {
        // Legacy modes:  No DBR, no bank wrap
        printf("*** ERROR - 24 bit operand in legacy mode! ***");
        ea = 0xFFFFFFFF;        
    }
    printf("OP_ABS_X_L($%08X)", ea); 
    return ea;      
}

uint32_t calc_zp(void)
{
    uint32_t ea;
    uint16_t dpr;
    uint8_t base;
    
    base = get_ir_indexed(1);
    if (is_65816()) {
        dpr = get_dpr();
        if (IS_EMU()) {
            // In emulation mode, page wrap-around
            ea = dpr + base;
        } else {
            // No page wrap around
            ea = base;
        }
    } else {
        ea = base;      // 0x00 + 8 bit operand
    }
    ea = dpr + base;    
    // FIXME: consider page crossings by mode 
    printf("OP_ZP($%08X)", ea);
    return ea;  
}

uint32_t calc_zp_x(void)
{
    uint32_t ea;
    uint16_t dpr;
    uint8_t base;
    
    base = get_ir_indexed(1);
    if (is_65816()) {
        /* 65816 */
        dpr = get_dpr();
        if (IS_EMU()) {
            /* Emulation mode */
            ea = dpr + base + (cpu_state.X & 0xFF);
        } else {
            if (GET_FLAG(X_FLAG)) {
                ea = dpr + base + (cpu_state.X & 0xFF);
            } else {
                ea = dpr + base + cpu_state.X;
            }
        }
    } else {
        // Legacy 6502 or 65C02
        ea = (base + (cpu_state.X & 0xFF)) & 0xFF;  // wraps
    }
    printf("OP_ZP_X($%08X)", ea);
    return ea;
}

uint32_t calc_zp_xi(void)
{
    uint32_t ea;
    uint8_t lsb, msb;
    
    ea = calc_zp_x();   // Get the EA of the EA
    lsb = cpu_read(ea);
    msb = cpu_read(ea+1);
    if (is_65816()) {
        if (IS_EMU()) {
            ea = (msb << 8) | lsb;
        } else {
            ea = get_pbr() | (msb << 8) | lsb;
        }
    } else {
        // Legacy CPU
        ea = (msb << 8) | lsb;
    }
    printf("OP_ZP_XI($%08X)", ea);
    return ea;
}

uint32_t calc_zp_y(void)
{
    uint32_t ea;
    uint16_t dpr;
    uint8_t base;
    
    base = get_ir_indexed(1);
    if (is_65816()) {
        /* 65816 */
        dpr = get_dpr();
        if (IS_EMU()) {
            /* Emulation mode */
            ea = dpr + base + (cpu_state.Y & 0xFF);
        } else {
            if (GET_FLAG(X_FLAG)) {
                ea = dpr + base + (cpu_state.Y & 0xFF);
            } else {
                ea = dpr + base + cpu_state.Y;
            }
        }
    } else {
        // Legacy 6502 or 65C02
        ea = (base + (cpu_state.Y & 0xFF)) & 0xFF;  // wraps
    }
    printf("OP_ZP_X($%08X", ea);
    return ea;
}

uint32_t calc_rel(void)
{
    uint32_t ea;
    uint32_t base;
    uint8_t offset;
    
    offset = get_ir_indexed(1); // SIGNED offset.  Handle accordingly
    
    if (is_65816()) {
        if (IS_EMU()) {
            base = (cpu_state.PC & 0xFFFF);
            if (offset < 0x80) {
                // Forward reference
                ea = (base + offset + 2) & 0xFFFF;  // (real PC has advanced by 2 since fetch)
            } else {
                // Reverse reference
                ea = (base - (0x100 - offset) + 2) & 0x00FFFF;
            }
        } else {
            base = (get_pbr() << 16) | (cpu_state.PC & 0x00FFFFFF);
            if (offset < 0x80) {
                // Forward reference
                ea = (base + offset + 2) & 0x00FFFFFF;  // (real PC has advanced by 2 since fetch)
            } else {
                // Reverse reference
                ea = (base - (0x100 - offset) + 2) & 0x00FFFFFF;
            }
        }
    } else {
        // Legacy CPU 
        base = cpu_state.PC & 0xFFFF;
        if (offset < 0x80) {
            // Forward reference
            ea = (base + offset + 2) & 0xFFFF;  // (real PC has advanced by 2 since fetch)
        } else {
            // Reverse reference
            ea = (base - (0x100 - offset) + 2) & 0x00FFFF;
        }
    }
    printf("OP_REL(%08X)", ea);
    return ea;
}

uint32_t calc_rel_l(void)
{
    uint32_t ea;
    uint32_t base;
    uint8_t offset;
    uint8_t lsb, msb;
    
    lsb = get_ir_indexed(1);    // SIGNED offset.  Handle accordingly
    msb = get_ir_indexed(2);    // MSB of 16 bit offset
    offset = (msb << 8) | lsb;
    
    if (is_65816()) {
        if (IS_EMU()) {
            base = (cpu_state.PC & 0xFFFF);
            if (offset < 0x8000) {
                // Forward reference
                ea = (base + offset + 3) & 0xFFFF;  // (real PC has advanced by 3 since fetch)
            } else {
                // Reverse reference
                ea = (base - (0x10000 - offset) + 3) & 0x00FFFF;
            }
        } else {
            base = (get_pbr() << 16) | (cpu_state.PC & 0x00FFFFFF);
            if (offset < 0x8000) {
                // Forward reference
                ea = (base + offset + 3) & 0x00FFFFFF;  // (real PC has advanced by 2 since fetch)
            } else {
                // Reverse reference
                ea = (base - (0x10000 - offset) + 3) & 0x00FFFFFF;
            }
        }
    } else {
        // Legacy CPU 
        base = cpu_state.PC & 0xFFFF;
        if (offset < 0x8000) {
            // Forward reference
            ea = (base + offset + 3) & 0xFFFF;  // (real PC has advanced by 2 since fetch)
        } else {
            // Reverse reference
            ea = (base - (0x10000 - offset) + 3) & 0x00FFFF;
        }
    }
    printf("OP_REL_L(%08X)", ea);
    return ea;
}

uint32_t calc_EA(void)
{
    uint32_t ea = 0xFFFFFFFF;
    
    switch((int) get_ir_addr_mode()) {
    case OP_NONE:
        printf("OP_NONE");
        ea = calc_noEA();
        break;
        
    case OP_A:
        printf("OP_A");
        ea = calc_noEA();
        break;

    case OP_IMM:
        /* No EA */
        printf("OP_IMM");
        ea = calc_noEA();
        break;

    case OP_ABS:
        ea = calc_abs();
        break;

    case OP_ABS_L:
        ea = calc_abs_l();
        break;
    
    case OP_ABS_X:
        ea = calc_abs_x();
        break;
    
    case OP_ABS_Y:
        ea = calc_abs_y();
        break;

    case OP_ABS_X_L:
        ea = calc_abs_x_l();
        break;

    case OP_ZP:
        ea = calc_zp();
        break;
    
    case OP_ZP_X:
        ea = calc_zp_x();
        break;

    case OP_ZP_Y:
        ea = calc_zp_y();
        break;
        
    case OP_REL:
        ea = calc_rel();
        break;
    
    case OP_REL_L:
        ea = calc_rel_l();
        break;
    
    case OP_ZP_XI:
        ea = calc_zp_xi();
        break;

    case OP_ZP_IY:
        printf("OP_ZP_IY");
        break;

    case OP_ZP_IND_L:
        printf("OP_ZP_IND_L");
        break;

    case OP_ZP_IND:
        printf("OP_ZP_IND");
        break;

    case OP_ZP_IY_L:
        printf("OP_ZP_IY_L");
        break;

    case OP_SR:
        printf("OP_SR");
        break; 

    case OP_SR_IY:
        printf("OP_SR_IY");
        break;

    case OP_ABS_IND:
        printf("OP_ABS_IND");
        break; 
    
    case OP_ABS_IND_L:
        printf("OP_ABS_IND_L");
        break;

    case OP_ABS_X_IND:
        printf("OP_ABS_X_IND");
        break;

    case OP_STACK:
        if ((get_cpu_type() == CPU_65C02) || (get_cpu_type() == CPU_6502) ||
            (cpu_state.em == 1)) {
            ea = (cpu_state.SP & 0x00FF) | 0x0100;
            printf("OP_STACK(%04X)", ea);
        } else {
            printf("OP_STACK(%04X)", cpu_state.SP);
        }
        break;
        
        
    case OP_2OPS:
        printf("OP_2OPS");
        break;
        
    default:
        printf("================ FATAL CODE ERROR:  UNIMPLEMENTED ADDRESS MODE!!!! ===========");
        break;
    } // switch address mode
    return ea;
}


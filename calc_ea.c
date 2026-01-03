#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "disasm.h"
#include "sim.h"


void calc_ea_noEA (void)
{
    // Since addresses are at most 24 bits, returning -1 
    // can't be confused with a valid effective address
    set_EA(0xFFFFFFFF);  // > 24 bits means no EA applies
}

// There are all sorts of optimization opportunities below!
// Intentionally avoided until all corner cases (page crossing behavior 
// (wrap, advance), 
// bank crossing behavior(wrap advance), cycle count adjustments &
// all other fine details must be fully worked through
// on all the addressing modes before it's worth trying to
// consolidate and optimize.  
// Get it working right first, then tighten it up.   
void calc_ea_abs (void)
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
    set_EA(ea);
}

void calc_ea_abs_ind (void)
{
    uint32_t ea;
    uint16_t base;
    uint8_t lsb, msb;
    
    calc_ea_abs();
    ea = get_EA();
    lsb = cpu_read(ea);
    msb = cpu_read(ea + 1);
    base = (msb << 8) | lsb;
    
    if (is_65816()) {
        if (IS_EMU()) {
            ea = base;
        } else {
            ea = (get_pbr() << 16) | base;
        }
    } else {
        ea = base;
        // legacy
    }     
    set_EA(ea);    
}

void calc_ea_abs_ind_l (void)
{
    uint32_t ea;
    uint8_t lsb, msb, page;

    calc_ea_abs();
    ea = get_EA();
    lsb = cpu_read(ea);
    msb = cpu_read(ea + 1);
    page = cpu_read(ea + 2);
    
    if (is_65816()) {
        ea = (page << 16) | (msb << 8) | lsb;
    } else {
        // legacy
        printf("*** ERROR: Not supported on legacy CPU! ***");
        ea = 0xFFFFFFFF;
    }   
    set_EA(ea);
}
    
void calc_ea_abs_l (void)
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
    set_EA(ea);
}

void calc_ea_abs_x (void)
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
    set_EA(ea);      
}

void calc_ea_abs_x_ind (void)
{
    uint32_t ea;

    calc_ea_abs_x();
    ea = get_EA();
    uint8_t lsb, msb, page;
    
    lsb = cpu_read(ea);
    msb = cpu_read(ea + 1);
    page = cpu_read(ea + 2);
    
    ea = (page << 16) | (msb << 8) | lsb;
    set_EA(ea);
}       
        
void calc_ea_abs_y (void)
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
    set_EA(ea);      
}

void calc_ea_abs_x_l (void)
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
    set_EA(ea);      
}

void calc_ea_zp (void)
{
    uint32_t ea;
    uint16_t dpr;
    uint8_t offset;
    
    offset = get_ir_indexed(1);
    if (is_65816()) {
        dpr = get_dpr();
        if (IS_EMU()) {
            // In emulation mode, page wrap-around
            ea = dpr + offset;
        } else {
            // No page wrap around
            ea = dpr + offset;
        }
    } else {
        ea = offset;      // 0x00 + 8 bit operand
    }    
    // FIXME: consider page crossings by mode 
    set_EA(ea);  
}


void calc_ea_zp_ind (void)
{
    uint32_t ea;
    uint16_t base;
    uint8_t lsb, msb, page;
    
    calc_ea_zp();
    ea = get_EA();
    lsb = cpu_read(ea);
    msb = cpu_read(ea + 1);
    base = (msb << 8) | lsb;
    page = get_pbr();
    if (is_65816()) {
        if (IS_EMU()) {
            ea = base;
        } else {
            ea = (page << 16) | base;
        }
    } else {
        // Legacy CPU
        ea = (page << 16) | base;
    }
    set_EA(ea);
}

void calc_ea_zp_ind_l (void)
{
    uint32_t ea;
    uint8_t lsb, msb, page;
    
    calc_ea_zp();
    ea = get_EA();
    lsb = cpu_read(ea);
    msb = cpu_read(ea + 1);
    page = cpu_read(ea + 2);
    
    if (is_65816()) {
        ea = (page << 16) | (msb << 8) | lsb;
    } else {
        // Legacy CPU
        printf("*** ERROR: not supported by selected CPU! ***");
        ea = 0xFFFFFFFF;
    }
    set_EA(ea);
}

void calc_ea_zp_iy_l (void)
{
    uint32_t ea;
    
    calc_ea_zp_ind_l();
    ea = get_EA();
    if (is_65816()) {
        if (IS_EMU()) {
            ea = ea + (cpu_state.Y & 0xFF);
        } else {
            if (GET_FLAG(X_FLAG)) {
                ea = ea + (cpu_state.Y & 0xFF);
            } else {
                ea = ea + cpu_state.Y;
            }
        }
    } else {
        // Legacy CPU
        printf("*** ERROR: not supported by selected CPU! ***");
        ea = 0xFFFFFFFF;
    }
    set_EA(ea);
}

void calc_ea_zp_x (void)
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
    set_EA(ea);
}

void calc_ea_zp_xi (void)
{
    uint32_t ea;
    uint8_t lsb, msb;
    
    calc_ea_zp_x();   // Get the EA of the EA
    ea = get_EA();
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
    set_EA(ea);
}

void calc_ea_zp_y (void)
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
    set_EA(ea);
}

void calc_ea_zp_iy (void)
{
    uint32_t ea;
    uint16_t base;
    uint8_t lsb, msb, page;
    
    calc_ea_zp();         // Address where pointer resides in direct page
    ea = get_EA();
    lsb = cpu_read(ea);     // Get pointer LSB
    msb = cpu_read(ea+1);   // Get pointer MSB
    base = (msb << 8) | lsb;
    
    if (is_65816()) {
        page = get_pbr();
        if (IS_EMU()) {
            ea = base + (cpu_state.Y & 0xFF);
        } else {
            if (GET_FLAG(X_FLAG)) {
                ea = ((page << 16) | base) + (cpu_state.Y & 0xFF);
            } else {
                ea = ((page << 16) | base) + cpu_state.Y;
            }
        }
    } else {
        // Legacy CPU
        ea = base + (cpu_state.Y & 0xFF);
    }
    set_EA(ea);
}
        
void calc_ea_rel (void)
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
    set_EA(ea);
}

void calc_ea_rel_l (void)
{
    uint32_t ea;
    uint32_t base;
    uint16_t offset;
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
    set_EA(ea);
}

void calc_ea_stack (void)
{
    uint32_t ea;
    
    if (is_65816()) {
        if (IS_EMU()) {
            ea = 0x100 | (cpu_state.SP & 0xFF);
        } else {
            ea = cpu_state.SP;
        }
    } else {
        // Legacy CPUs
        ea = 0x100 | (cpu_state.SP & 0xFF);
    }
    set_EA(ea);
}

void calc_ea_sr (void)
{
    uint32_t ea;
    uint16_t sp;
    uint8_t offset;
    
    offset = get_ir_indexed(1);
    if (is_65816()) {
        if (IS_EMU()) {
            sp = 0x0100 | (((cpu_state.SP & 0xFF) + offset) & 0xFF);
        } else {
            sp = cpu_state.SP + offset;
        }
        ea = sp;
    } else {
        // Legacy CPU
        printf("*** ERROR: Unsupported addressing mode for CPU! ***");
        ea = 0xFFFFFFFF;
    }
    set_EA(ea);
}

void calc_ea_sr_iy (void)
{
    uint32_t ea;
    uint8_t lsb, msb;
    
    calc_ea_sr();
    ea = get_EA();
    lsb = cpu_read(ea);
    msb = cpu_read(ea+1);
    ea = (msb << 8) | (lsb);
    
    if (is_65816()) {
        if (IS_EMU()) {
            ea = ea + (cpu_state.Y & 0xFF);
        } else {
            if (GET_FLAG(X_FLAG)) {
                ea = ea + (cpu_state.Y & 0xFF);
            } else {
                ea = ea + cpu_state.Y;
            }
        }
    } else {
        // Legacy CPU
        printf("*** ERROR: unsupported addressing mode for CPU! ***");
    }
    set_EA(ea);
}

// Replace with a jump table or table-based call.
void print_EA (void)
{
    switch((int) get_ir_addr_mode()) {
    case OP_NONE:
        printf("OP_NONE($%08X)", get_EA());  
        calc_ea_noEA();   // OK
        break;
        
    case OP_A:
        printf("OP_A($%08X)", get_EA()); 
        calc_ea_noEA();   // OK
        break;

    case OP_IMM:
        /* No EA */
        printf("OP_IMM($%08X)", get_EA());
        calc_ea_noEA();   // OK
        break;

    case OP_ABS:
        printf("OP_ABS($%08X)", get_EA());
        calc_ea_abs();    // FP OK
        break;

    case OP_ABS_IND:
        printf("OP_ABS_IND($%08X)", get_EA());
        calc_ea_abs_ind();    // FP OK
        break; 
     
    case OP_ABS_IND_L:
        printf("OP_ABS_IND_L($%08X)", get_EA());
        calc_ea_abs_ind_l();  // FP OK
        break;

    case OP_ABS_L:
        printf("OP_ABS_L($%08X)", get_EA());
        calc_ea_abs_l();      // FP OK
        break;
    
    case OP_ABS_X:
        printf("OP_ABS_X($%08X)", get_EA());
        calc_ea_abs_x();      // FP OK
        break;
        
    case OP_ABS_X_IND:
        printf("OP_ABS_X_IND($%08X)", get_EA());
        calc_ea_abs_x_ind();  // FP OK
        break;
    
    case OP_ABS_Y:
        printf("OP_ABS_Y($%08X)", get_EA());
        calc_ea_abs_y();      // FP OK
        break;

    case OP_ABS_X_L:
        printf("OP_ABS_X_L($%08X)", get_EA());
        calc_ea_abs_x_l();    // FP OK
        break;

    case OP_ZP:
        printf("OP_ZP($%08X)", get_EA());
        calc_ea_zp();         // FP OK    
        break;

    case OP_ZP_IND:
        printf("OP_ZP_IND($%08X)", get_EA());
        calc_ea_zp_ind();     //FP OK     
        break;

    case OP_ZP_IND_L:
        printf("OP_ZP_IND_L($%08X)", get_EA());
        calc_ea_zp_ind_l();   // FP OK
        break;    
    
    case OP_ZP_IY_L:
        printf("OP_ZP_IY_L($%08X)", get_EA());
        calc_ea_zp_iy_l();    // FP OK
        break;
        
    case OP_ZP_X:
        printf("OP_ZP_X($%08X)", get_EA());
        calc_ea_zp_x();       // FP OK
        break;

    case OP_ZP_Y:
        printf("OP_ZP_Y($%08X)", get_EA());
        calc_ea_zp_y();       // FP OK
        break;
        
    case OP_REL:
        printf("OP_REL($%08X)", get_EA());
        calc_ea_rel();        // FP OK
        break;
    
    case OP_REL_L:
        printf("OP_REL_L($%08X)", get_EA());
        calc_ea_rel_l();      // FP OK
        break;
    
    case OP_ZP_XI:
        printf("OP_ZP_XI($%08X)", get_EA());
        calc_ea_zp_xi();      // FP OK
        break;

    case OP_ZP_IY:
        printf("OP_ZP_IY($%08X)", get_EA());
        calc_ea_zp_iy();      // FP OK
        break;

    case OP_SR:
        printf("OP_SR($%08X)", get_EA());
        calc_ea_sr();         // FP OK
        break; 

    case OP_SR_IY:
        printf("OP_SR_IY($%08X)", get_EA());
        calc_ea_sr_iy();      // FP OK
        break;

    case OP_STK:
        printf("OP_STK($%08X)", get_EA());
        calc_ea_stack();
        break;
        
    default:
        printf("================ FATAL CODE ERROR:  UNIMPLEMENTED ADDRESS MODE!!!! ===========");
        exit(-1);
    } // switch address mode
}
 
// Replace with a jump table or table-based call.
void calc_EA (void)
{
    switch((int) get_ir_addr_mode()) {
        
    case OP_NONE:  
        calc_ea_noEA();   // OK
        break;
        
    case OP_A: 
        calc_ea_noEA();   // OK
        break;

    case OP_IMM:
        /* No EA */
        calc_ea_noEA();   // OK
        break;

    case OP_ABS:
        calc_ea_abs();    // FP OK
        break;

    case OP_ABS_IND:
        calc_ea_abs_ind();    // FP OK
        break; 
     
    case OP_ABS_IND_L:
        calc_ea_abs_ind_l();  // FP OK
        break;

    case OP_ABS_L:
        calc_ea_abs_l();      // FP OK
        break;
    
    case OP_ABS_X:
        calc_ea_abs_x();      // FP OK
        break;
        
    case OP_ABS_X_IND:
        calc_ea_abs_x_ind();  // FP OK
        break;
    
    case OP_ABS_Y:
        calc_ea_abs_y();      // FP OK
        break;

    case OP_ABS_X_L:
        calc_ea_abs_x_l();    // FP OK
        break;

    case OP_ZP:
        calc_ea_zp();         // FP OK    
        break;

    case OP_ZP_IND:
        calc_ea_zp_ind();     //FP OK     
        break;

    case OP_ZP_IND_L:
        calc_ea_zp_ind_l();   // FP OK
        break;    
    
    case OP_ZP_IY_L:
        calc_ea_zp_iy_l();    // FP OK
        break;
        
    case OP_ZP_X:               
        calc_ea_zp_x();       // FP OK
        break;

    case OP_ZP_Y:
        calc_ea_zp_y();       // FP OK
        break;
        
    case OP_REL:
        calc_ea_rel();        // FP OK
        break;
    
    case OP_REL_L:
        calc_ea_rel_l();      // FP OK
        break;
    
    case OP_ZP_XI:
        calc_ea_zp_xi();      // FP OK
        break;

    case OP_ZP_IY:
        calc_ea_zp_iy();      // FP OK
        break;

    case OP_SR:
        calc_ea_sr();         // FP OK
        break; 

    case OP_SR_IY:
        calc_ea_sr_iy();      // FP OK
        break;

    case OP_STK:
        calc_ea_stack();
        break;
        
    default:
        printf("================ FATAL CODE ERROR:  UNIMPLEMENTED ADDRESS MODE!!!! ===========");
        exit(-1);
    } // switch address mode
}


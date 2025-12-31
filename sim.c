#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "disasm.h"
#include "sim.h"




const uint16_t VEC_RESET = 0xFFFC;
cpu_state_t cpu_state;

void set_flags(uint8_t fset_mask)
{
	cpu_state.flags |= fset_mask;
}

void clr_flags(uint8_t fres_mask)
{
	cpu_state.flags &= ~fres_mask;
}

uint8_t get_flags(void)
{
	return cpu_state.flags;
}

void set_emu(BOOL emu_mode)
{
	if (emu_mode) {
		cpu_state.em = 0x1;
	} else {
		cpu_state.em = 0x0;
	}
}

uint8_t get_emu(void)
{
	return cpu_state.em;
}

void init_cpu(void)
{
	uint16_t new_pc;
	
	set_emu(TRUE);		// Comes out of reset in EMU mode
	cpu_state.A.AX = 0;
	cpu_state.X = 0;
	cpu_state.Y = 0;
	cpu_state.SP = 0x01FF;	// Not really!
	cpu_state.DBR = 0;
	cpu_state.PBR = 0;
	cpu_state.DPR = 0x0000;	// Probably
	new_pc = (cpu_read(VEC_RESET+1) & 0xFF) << 8;
	new_pc |= (cpu_read(VEC_RESET) & 0xFF);
	cpu_state.PC = new_pc;
}

int main(void)
{	
	uint32_t start_address;
	uint32_t end_address;
	
	
    init_mem();
    init_cpu();
    alloc_block(0x7F00, 0x7F1F, handler_io_unimplemented);	// XBUS0 (not implmemented)
    alloc_block(0x7F20, 0x7F3F, handler_io_unimplemented);	// XBUS1 (not implemented)
    alloc_block(0x7F40, 0x7F5F, handler_io_unimplemented);	// XBUS2 (not implemented)
    alloc_block(0x7F60, 0x7F7F, handler_io_unimplemented);	// XBUS3 (not implemented)
    alloc_block(0x7F80, 0x7F9F, handler_acia);	// ACIA
    alloc_block(0x7FA0, 0x7FBF, handler_pia);	// PIA
    alloc_block(0x7FC0, 0x7FDF, handler_via1);	// VIA
    alloc_block(0x7FE0, 0x7FFF, handler_via2);	// USB VIA
    alloc_block(0x0, 0x7EFF, handler_ram);		// RAM
    alloc_block(0x8000, 0xFFFF, handler_flash);	// FLASH
    print_block_list();
 
    load_srec("allops_m0x0.s19", &start_address, &end_address);
    set_op_mode(CPU_MODE_M0X0);
    printf("**** M0X0 sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m0x1.s19", &start_address, &end_address);
    set_op_mode(CPU_MODE_M0X1);
    printf("****  M0X1  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m1x0.s19", &start_address, &end_address);
    set_op_mode(CPU_MODE_M1X0);
    printf("****  M1X0  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
  
	load_srec("allops_m1x1.s19", &start_address, &end_address);
    set_op_mode(CPU_MODE_M1X1);
    printf("****  M1X1  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_65c02.s19", &start_address, &end_address);
    set_op_mode(CPU_MODE_CMOS_6502);
    printf("****  65c02  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_6502.s19", &start_address, &end_address);
    set_op_mode(CPU_MODE_NMOS_6502);
    printf("****  6502  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    exit(0);
}


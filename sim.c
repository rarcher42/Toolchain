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
uint8_t cpu_type;       // 0 = 65816
                        // 1 = 6502
                        // 2 = 65c02

const uint8_t N_FLAG = (1 << 7);
const uint8_t V_FLAG = (1 << 6);
const uint8_t M_FLAG = (1 << 5);
const uint8_t X_FLAG = (1 << 4);
const uint8_t D_FLAG = (1 << 3);
const uint8_t I_FLAG = (1 << 2);
const uint8_t Z_FLAG = (1 << 1);
const uint8_t C_FLAG = 0x01;

uint8_t get_cpu_type(void)
{
	return cpu_type;
}

void SET_FLAG (uint8_t fset_mask)
{
    cpu_state.flags |= fset_mask;
}

void CLR_FLAG (uint8_t fres_mask)
{
    cpu_state.flags &= ~fres_mask;
}

uint8_t GET_FLAGS(void)
{
    return cpu_state.flags;
}

uint8_t GET_FLAG(uint8_t flag)
{
	if (cpu_state.flags & flag) {
		return 1;
	}
	return 0;
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
    
    set_emu(TRUE);      // Comes out of reset in EMU mode
    cpu_state.A.AX = 0;
    cpu_state.X = 0;
    cpu_state.Y = 0;
    cpu_state.SP = 0x01FF;  // Not really!
    cpu_state.DBR = 0;
    cpu_state.PBR = 0;
    cpu_state.DPR = 0x0000; // Probably
    new_pc = (cpu_read(VEC_RESET+1) & 0xFF) << 8;
    new_pc |= (cpu_read(VEC_RESET) & 0xFF);
    cpu_state.PC = new_pc;
    cpu_type = 0;       // Not 6502, 65c02 at this time
}

int main(void)
{   
    uint32_t start_address;
    uint32_t end_address;
    
    
    init_mem();
    alloc_block(0x7F00, 0x7F1F, handler_io_unimplemented);  // XBUS0 (not implmemented)
    alloc_block(0x7F20, 0x7F3F, handler_io_unimplemented);  // XBUS1 (not implemented)
    alloc_block(0x7F40, 0x7F5F, handler_io_unimplemented);  // XBUS2 (not implemented)
    alloc_block(0x7F60, 0x7F7F, handler_io_unimplemented);  // XBUS3 (not implemented)
    alloc_block(0x7F80, 0x7F9F, handler_acia);  // ACIA
    alloc_block(0x7FA0, 0x7FBF, handler_pia);   // PIA
    alloc_block(0x7FC0, 0x7FDF, handler_via1);  // VIA
    alloc_block(0x7FE0, 0x7FFF, handler_via2);  // USB VIA
    alloc_block(0x0, 0x7EFF, handler_ram);      // RAM
    alloc_block(0x8000, 0xFFFF, handler_flash); // FLASH
    print_block_list();

    init_cpu(); 
    load_srec("allops_m0x0.s19", &start_address, &end_address);
    // set_op_mode(CPU_MODE_M0X0)
    CLR_FLAG(M_FLAG | X_FLAG);
    cpu_type = 0x0;
    printf("**** M%dX%d sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m0x1.s19", &start_address, &end_address);
    // set_op_mode(CPU_MODE_M0X1)
    CLR_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    cpu_type = 0;
    printf("****  M%dX%d  sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m1x0.s19", &start_address, &end_address);
    // set_op_mode(CPU_MODE_M1X0);
    SET_FLAG(M_FLAG);
    CLR_FLAG(X_FLAG);
    cpu_type = 0;
    printf("****  M%dX%d  sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
  
    load_srec("allops_m1x1.s19", &start_address, &end_address);
    // set_op_mode(CPU_MODE_M1X1);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    cpu_type = 0;
    printf("****  M%dX%d  sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_65c02.s19", &start_address, &end_address);
    // set_op_mode(CPU_MODE_CMOS_6502);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    cpu_type = 2;
    printf("****  65c02  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_6502.s19", &start_address, &end_address);
    // set_op_mode(CPU_MODE_NMOS_6502);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    cpu_type = 1;
    printf("****  6502  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    exit(0);
}


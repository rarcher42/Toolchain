#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "disasm.h"
#include "sim.h"

cpu_state_t cpu_state;
cpu_dynamic_metadata_t cpu_dynamic_metadata;
cpu_static_metadata_t cpu_static_metadata;


const uint16_t VEC_RESET = 0xFFFC;


uint8_t get_cpu_type(void)
{
    return cpu_static_metadata.cpu_type;
}

void set_cpu_type(uint8_t ct)
{
    cpu_static_metadata.cpu_type = ct;
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

void SET_EMU(BOOL emu_mode)
{
    if (emu_mode) {
        cpu_state.em = 0x1;
    } else {
        cpu_state.em = 0x0;
    }
}

uint8_t GET_EMU(void)
{
    return cpu_state.em;
}

void init_cpu(void)
{
    uint16_t new_pc;
    
    SET_EMU(FALSE);         // FUBAR for testing disasm only, should be TRUE
    set_cpu_type(CPU_65816);       // Not 6502, 65c02 at this time
    cpu_state.A.C = 0;
    cpu_state.X = 0;
    cpu_state.Y = 0;
    cpu_state.SP = 0x7EFF;  // Not really!
    cpu_state.DBR = 0;
    cpu_state.PBR = 0;
    cpu_state.DPR = 0x0000; // Probably
    new_pc = (cpu_read(VEC_RESET+1) & 0xFF) << 8;
    new_pc |= (cpu_read(VEC_RESET) & 0xFF);
    cpu_state.PC = new_pc;
}

uint32_t calc_EA(void)
{
    address_mode_t addr_mode;
    uint8_t oplen;
    uint16_t ea;
    
    addr_mode = cpu_dynamic_metadata.addr_mode;
    oplen = cpu_dynamic_metadata.oplen;
    
    switch((int) addr_mode) {
    case OP_NONE:
        printf("OP_NONE(null)");
        break;
        
    case OP_A:
        if (GET_FLAG(M_FLAG)) {
            printf("OP_A(%02X)", cpu_state.A.AL);
        } else {
            printf("OP_A(%04X)", cpu_state.A.C);
        }
        break;

    case OP_IMM:
        if (oplen == 3) 
            ;
        else
            ;
        printf("OP_IMM");
        break;

    case OP_ABS:
        printf("OP_ABS");
        break;

    case OP_ZP:
        printf("OP_ZP");
        break;
    
    case OP_ABS_L:
        printf("OP_ABS_L");
        break;
    
    case OP_REL:
        printf("OP_REL");
        break;
    
    case OP_REL_L:
        printf("OP_REL_L");
        break;
    
    case OP_ZP_XI:
        printf("OP_ZX_XI");
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

    case OP_ZP_X:
        printf("OP_ZP_X");
        break;

    case OP_ZP_Y:
        printf("OP_ZP_Y");
        break;

    case OP_ABS_X:
        printf("OP_ABS_X");
        break;

    case OP_ABS_X_L:
        printf("OP_ABS_X_L");
        break;

    case OP_ABS_Y:
        printf("OP_ABS_Y");
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
        if ((cpu_static_metadata.cpu_type == CPU_65c02) || (cpu_static_metadata.cpu_type == CPU_6502) ||
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
    return 0x00000000;  // Fill in later
}

void print_fetchbuffer(void)
{
    int i;
    
    printf("ir=");
    for (i = 0; i < cpu_dynamic_metadata.oplen; i++) {
        printf("%02X ", cpu_dynamic_metadata.ir[i]);
    }
    printf("\n");   
}

void cpu_fetch(uint32_t addr)
{
    uint8_t op;
    int i;
    
    op = cpu_read(addr);
    cpu_dynamic_metadata.ir[0] = op;
    cpu_dynamic_metadata.oplen = get_oplen(op);
    cpu_dynamic_metadata.addr_mode = get_addr_mode(op);
    if (cpu_dynamic_metadata.oplen > 1) {
        for (i = 1; i < cpu_dynamic_metadata.oplen; i++) {
            cpu_dynamic_metadata.ir[i] = cpu_read(addr+i);
        }
    }
    // print_fetchbuffer();
}

uint8_t get_ir_opcode(void)
{
    return cpu_dynamic_metadata.ir[0];
}

uint8_t get_ir_oplen(void)
{
    return cpu_dynamic_metadata.oplen;
}

address_mode_t get_ir_addr_mode(void)
{
    return cpu_dynamic_metadata.addr_mode;
}

uint8_t get_ir_indexed(uint8_t index)
{
    return cpu_dynamic_metadata.ir[index];
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
    cpu_state.A.C = 0x6502;
    load_srec("allops_m0x0.s19", &start_address, &end_address);
    CLR_FLAG(M_FLAG | X_FLAG);
    set_cpu_type(CPU_65816);
    printf("**** M%dX%d sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m0x1.s19", &start_address, &end_address);
    CLR_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    set_cpu_type(CPU_65816);
    printf("****  M%dX%d  sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m1x0.s19", &start_address, &end_address);
    SET_FLAG(M_FLAG);
    CLR_FLAG(X_FLAG);
    set_cpu_type(CPU_65816);
    printf("****  M%dX%d  sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
  
    load_srec("allops_m1x1.s19", &start_address, &end_address);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    set_cpu_type(CPU_65816);
    printf("****  M%dX%d  sa = %08X, ea=%08X ***** \n", GET_FLAG(M_FLAG), GET_FLAG(X_FLAG), start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_65c02.s19", &start_address, &end_address);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    set_cpu_type(CPU_65c02);
    printf("****  65c02  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_6502.s19", &start_address, &end_address);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    set_cpu_type(CPU_6502);
    printf("****  6502  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    exit(0);
}


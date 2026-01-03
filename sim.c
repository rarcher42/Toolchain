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

cpu_state_t cpu_state;
cpu_dynamic_metadata_t cpu_dynamic_metadata;
cpu_static_metadata_t cpu_static_metadata;


const uint16_t VEC_RESET = 0xFFFC;


uint8_t get_cpu_type (void)
{
    return cpu_static_metadata.cpu_type;
}

BOOL is_65816 (void)
{
    if ((cpu_static_metadata.cpu_type == CPU_6502) ||
        (cpu_static_metadata.cpu_type == CPU_65C02)) {
        return FALSE;
    }
    return TRUE;
}

BOOL is_6502 (void)
{
	return (cpu_static_metadata.cpu_type == CPU_6502);
}

BOOL is_65C02 (void)
{
	return (cpu_static_metadata.cpu_type == CPU_65C02);
}


void set_cpu_type (uint8_t ct)
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

uint8_t GET_FLAGS (void)
{
    return cpu_state.flags;
}

uint8_t GET_FLAG (uint8_t flag)
{
    if (cpu_state.flags & flag) {
        return 1;
    }
    return 0;
}

void SET_EMU (BOOL emu_mode)
{
    if (emu_mode) {
        cpu_state.em = 0x1;
    } else {
        cpu_state.em = 0x0;
    }
}

uint8_t IS_EMU (void)
{
    return cpu_state.em;
}

uint8_t get_dbr (void) 
{
    return cpu_state.DBR;
}

uint8_t get_pbr (void)
{
    return cpu_state.PBR;
}

uint16_t get_dpr (void)
{
    return cpu_state.DPR;
}

void init_cpu (void)
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


void print_fetchbuffer (void)
{
    int i;
    
    printf("ir=");
    for (i = 0; i < cpu_dynamic_metadata.oplen; i++) {
        printf("%02X ", cpu_dynamic_metadata.ir[i]);
    }
    printf("\n");   
}

uint32_t make_linear_address(uint8_t bank, uint16_t pc)
{
    return (((bank & 0xFF) << 16) | (pc & 0xFFFF));
}

uint32_t get_cpu_address_linear(void)
{
	return make_linear_address(cpu_state.PBR, cpu_state.PC);
}

void put_cpu_address_linear(uint32_t address)
{
	uint8_t bank;
	uint16_t pc;
	
	bank = (address >> 16) & 0xFF;
	pc = (address & 0xFFFF);
	
	cpu_state.PBR = bank;
	cpu_state.PC = pc;
}

uint8_t get_ir_opcode (void)
{
    return cpu_dynamic_metadata.ir[0];
}

uint8_t get_ir_oplen (void)
{
    return cpu_dynamic_metadata.oplen;
}

void set_EA (uint32_t ea)
{
    cpu_dynamic_metadata.EA = ea;
}

uint32_t get_EA (void)
{
    return cpu_dynamic_metadata.EA;
}

address_mode_t get_ir_addr_mode (void)
{
    return cpu_dynamic_metadata.addr_mode;
}

uint8_t get_ir_indexed (uint8_t index)
{
    return cpu_dynamic_metadata.ir[index];
}

void cpu_fetch (void)
{
    uint32_t addr;
    uint8_t op;
    int i;
    
    addr = get_cpu_address_linear();
    op = cpu_read(addr);
    cpu_dynamic_metadata.ir[0] = op;
    cpu_dynamic_metadata.oplen = get_oplen(op);
    
    if (cpu_dynamic_metadata.oplen > 1) {
        for (i = 1; i < cpu_dynamic_metadata.oplen; i++) {
            cpu_dynamic_metadata.ir[i] = cpu_read(addr+i);
        }
    }
    // print_fetchbuffer();
}

void cpu_decode (void)
{
    uint8_t opcode;
    
    opcode = cpu_dynamic_metadata.ir[0];    // opcode
    cpu_dynamic_metadata.addr_mode = get_addr_mode(opcode);
    calc_EA();
}

// run one instruction
void cpu_execute (void)
{
    
}

int main (void)
{   
    uint32_t start_address;
    uint32_t end_address;
    
    init_vm();  // Create the infrastructure to support memory regions
    alloc_target_system_memory();   // Create the system memory blocks
    print_block_list();

    init_cpu(); 
    cpu_state.A.C = 0xABCD;
    cpu_state.DPR = 0x9000;     // HAL would approve
    cpu_state.PBR = 0x00;
    cpu_state.DBR = 0x00;
    cpu_state.X = 0x0101;
    cpu_state.Y = 0x0202;
    cpu_state.SP = 0x70FF;
    cpu_write(0xFFFC, 0x04);
    cpu_write(0xFFFD, 0x2A);    // SOL-20 magic entry point
    cpu_write(0x1234, 0x02);
    cpu_write(0x1235, 0x65);
    cpu_write(0x1236, 0x99);
    cpu_write(0x8081, 0x09);
    cpu_write(0x8082, 0x68);
    cpu_write(0x8083, 0x07);
    cpu_write(0x8181, 0x09);
    cpu_write(0x8182, 0x68);
    cpu_write(0x8183, 0xAA);
    cpu_write(0x9042, 0x08);
    cpu_write(0x9043, 0x80);
    cpu_write(0x42, 0x04);
    cpu_write(0x43, 0x40);
    
    cpu_write(0x33, 0x56);
    cpu_write(0x34, 0x34);
    cpu_write(0x35, 0x12);
    
    cpu_write(0x9012, 0x66);
    cpu_write(0x9013, 0x67);
    cpu_write(0x9014, 0x68);
    
    cpu_write(0x12, 0x57);
    cpu_write(0x13, 0x56);
    cpu_write(0x14, 0x55);
    
    
    cpu_write(0x9033, 0xEF);
    cpu_write(0x9034, 0xCD);
    cpu_write(0x9035, 0xAB);
    
    cpu_write(0x9131, 0x55);
    cpu_write(0x9132, 0xAA);
    cpu_write(0x9031, 0x55);
    cpu_write(0x9032, 0xAA);
    cpu_write(0x31, 0x55);
    cpu_write(0x32, 0xAA);
    
    cpu_write(0x7100, 0x02);
    cpu_write(0x7101, 0x18);
    
    load_srec("allops_m1x1.s19", &start_address, &end_address);
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    set_cpu_type(CPU_65816);
    SET_EMU(TRUE);
    printf("****  EMULATION  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    
    
    set_cpu_type(CPU_65816);
    SET_EMU(FALSE);
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
    set_cpu_type(CPU_65C02);
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


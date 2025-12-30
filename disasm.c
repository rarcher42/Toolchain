#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"

// Ugly legacy stuff - replace with structured memory with attributes
//
uint32_t start_address;	// Lowest address loaded by load_srec()
uint32_t end_address;	// Highest address loaded by load_srec()
uint32_t pc;
    // mode = 0x0 : M=0 X=0 E=0 
    // mode = 0x1 : M=0 X=1 E=0
    // mode = 0x2 : M=1 X=0 E=0
    // mode = 0x3 : M=1 X=1 E=0 or Emulation mode (no diff in disasm)
    // mode = 0x4 : Real 65c02 (no 65c816 at all)
    // mode = 0x5 : Real NMOS 6502
    // mode = 0xFF: not yet supported, E mode 65c816 (same as mode 3 for disasm)
uint8_t op_mode;

// end legacy stuff


address_mode_t get_addr_mode (uint8_t op)
{
    return opcode_table[op].adm;
}

uint8_t get_oplen (uint8_t op) 
{
    uint8_t sizeinfo;
    uint8_t oplen;

	sizeinfo = opcode_table[op].sizeinfo;
 
    if ((op_mode == CPU_MODE_NMOS_6502) && (sizeinfo & NOT_6502)) { 
        printf("Unimplemented NMOS 6502 opcode $%02X\n", op);
	return 0;    // Not implemented!
    }
    if ((op_mode == CPU_MODE_CMOS_6502) && (sizeinfo & NOT_65C02)) {
        printf("Unimplemented CMOS 65c02 opcode $%02X\n", op);
	return 0;    // Not implemented!
    }
    oplen = sizeinfo & 0x7;  // Extract length bits
   
    if (sizeinfo & M_ADDS) {
	// Instruction:  add 1 byte if M flag is set
        if ((op_mode == CPU_MODE_M0X0) || (op_mode == CPU_MODE_M0X1)) {
	        ++oplen;
		}
	}
    if (sizeinfo & X_ADDS)  {
        if ((op_mode == CPU_MODE_M0X0) || (op_mode == CPU_MODE_M1X0)) {
	        ++oplen;
		}
    }
    return oplen;
}

int disasm_one(uint32_t my_addr, char *outs)
{
    uint32_t val = 0;
    uint8_t op;
    uint8_t oplen;
    address_mode_t addr_mode;
    char param[20];

    outs[0] = (char) 0;
    param[0] = (char) 0;
    op = read_byte(my_addr);
    oplen = get_oplen(op);
    addr_mode = get_addr_mode(op);
    val = 0;
    if (oplen > 1) {
        val = from_hex(my_addr+1, oplen-1);
    }
    
    sprintf(outs, "%s ", opcode_table[op].ops);

    switch((int) addr_mode) {
    case OP_NONE:
        break;	// No operands, valid
    case OP_A:
        strcpy(param, "A");
        break;

    case OP_IMM:
        if (oplen == 3) 
            sprintf(param, "#$%04X ", val);
	else
            sprintf(param, "#$%02X ", val);
	break;

    case OP_ABS:
        sprintf(param, "$%04X ", val);
        break;

    case OP_ZP:
	sprintf(param, "$%02X ", val);
        break;
    
    case OP_ABS_L:
	sprintf(param, "$%06X ", val);
        break;
    
    case OP_REL:
        if (val > 0x7F)
            val = 0x100 - val;
        val = my_addr + 2 - val;
        sprintf(param, "$%04X ", val);
	break;
    
    case OP_REL_L:
        if (val > 0x7FFF)
            val = 0x10000 - val;
        val = my_addr + 3 - val;
        sprintf(param, "$%04X ", val);
	break;
	
    case OP_ZP_XI:
	sprintf(param, "($%02X,X) ", val);
        break;

    case OP_ZP_IY:
	sprintf(param, "($%02X),Y ", val);
        break;

    case OP_ZP_IND_L:
	sprintf(param, "[$%02X] ", val);
        break;

    case OP_ZP_IND:
	sprintf(param, "($%02X) ", val);
        break;

    case OP_ZP_IY_L:
	sprintf(param, "[$%02X],Y ", val);
        break;

    case OP_ZP_X:
	sprintf(param, "$%02X,X ", val);
        break;

    case OP_ZP_Y:
	sprintf(param, "$%02X,Y ", val);
        break;

    case OP_ABS_X:
	sprintf(param, "$%04X,X ", val);
        break;

    case OP_ABS_X_L:
	sprintf(param, "$%06X,X ", val);
        break;

    case OP_ABS_Y:
	sprintf(param, "$%04X,Y ", val);
        break;

    case OP_SR:
	sprintf(param, "$%02X,S ", val);
        break; 

    case OP_SR_IY:
	sprintf(param, "($%02X,S),Y ", val);
        break;

    case OP_ABS_IND:
	sprintf(param, "($%04X) ", val);
        break; 
    
    case OP_ABS_IND_L:
	sprintf(param, "[$%06X] ", val);
        break;

    case OP_ABS_X_IND:
	sprintf(param, "($%04X,X) ", val);
        break;

    case OP_2OPS:
	uint8_t a = from_hex(my_addr + 1, 1);
	uint8_t b = from_hex(my_addr + 2, 1);
	sprintf(param, "%02X,%02X ", a, b);
        break;
    default:
        printf("\nUNKNOWN addressing mode %d: aborting!\n", (int) addr_mode);
	exit(0);
    } // switch addr_mode
    strcat(outs, param);
    return oplen;
}

void disasm (uint32_t sa, uint32_t ea)
{
    uint32_t lpc;
    int i;
    uint8_t op_len;
    char outs[128];

    lpc = sa;
    while (lpc <= ea) {
	printf("%06X: ", lpc);
	op_len = get_oplen(read_byte(lpc));
        for (i = 0; i < 4; i++) {
            if (i < op_len) {
                printf("%02X ", read_byte(lpc+i));
	    } else {
	        printf("   ");
	    }
	}
    disasm_one(lpc, outs);
	if (op_len == 0) {
            printf("Invalid op-code: aborting disassembly!\n");
	    // This probably isn't what we want to do, usually.  But good
	    // to catch while running test suite where this should be 
	    // impossible
	    return;
	}
	printf("%s\n", outs);
	lpc += op_len;
    } 
}

int handler_via1 (void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	printf("called handler_via1($%08X, $%02X. wr=%d)\n", addr, data, wr);
	return 0;
}

int handler_via2 (void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	printf("called handler_via2($%08X, $%02X), wr=%d\n", addr, data, wr);
	return 0;
}

int handler_acia(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	printf("called handler_acia($%08X, $%02X), wr=%d\n", addr, data, wr);
	return 0;
}

int handler_pia(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	printf("called handler_pia($%08X, $%02X), wr=%d\n", addr, data, wr);
	return 0;
}

int handler_io_unimplemented(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	printf("called handler_io_unimplemented($%08X, $%02X)\n, wr=%d", addr, data, wr);
	return 0;
}


int handler_null(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	printf("called handler_null($%08X, $%02X), wr=%d\n", addr, data, wr);
	return 0;
}

int handler_ram(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	mem_block_descriptor_t *p;
	
	p = (mem_block_descriptor_t *) bdp;
	if (p == NULL) {
		printf("\nCannot locate RAM descriptor: R/W operation failed!\n");
		return -1;
	}
	if (wr) {
		//cprintf("Writing $%02X to RAM location %08X\n", data, addr);
		p->mem[addr - p->saddr] = data;
		return 0;
	} else {
		// printf("Reading from RAM location %08X\n", addr);
		return p->mem[addr - p->saddr];
	}
}

int handler_rom(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{

	mem_block_descriptor_t *p;
	
	p = (mem_block_descriptor_t *) bdp;
	if (p == NULL) {
		printf("\nCannot locate ROM descriptor: R/W operation failed!\n");
		return -1;
	}
	if (wr) {
		printf("Error:  cannot write to ROM location $%08X\n", addr);
		return 0;
	} else {
		printf("Reading from ROM location $%08X\n", addr);
		return p->mem[addr - p->saddr];
	}
}


int handler_flash(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
	return 0;
}



int main(void)
{	
    init_mem();
    alloc_block(0x7F00, 0x7F1F, handler_io_unimplemented);	// XBUS0 (not implmemented)
    alloc_block(0x7F20, 0x7F3F, handler_io_unimplemented);	// XBUS1 (not implemented)
    alloc_block(0x7F40, 0x7F5F, handler_io_unimplemented);	// XBUS2 (not implemented)
    alloc_block(0x7F60, 0x7F7F, handler_io_unimplemented);	// XBUS3 (not implemented)
    alloc_block(0x7F80, 0x7F9F, handler_acia);	// ACIA
    alloc_block(0x7FA0, 0x7FBF, handler_pia);		// PIA
    alloc_block(0x7FC0, 0x7FDF, handler_via1);	// VIA
    alloc_block(0x7FE0, 0x7FFF, handler_via2);	// USB VIA
    alloc_block(0x0, 0x7EFF, handler_ram);		// RAM
    alloc_block(0x8000, 0xFFFF, handler_flash);	// FLASH
    print_block_list();
    write_byte(0x0100, 0x31);
    write_byte(0x7FC0, 0x22);
 
    load_srec("allops_m0x0.s19");
  
    op_mode = CPU_MODE_M0X0;
    printf("**** M0X0 sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m0x1.s19");
    op_mode = CPU_MODE_M0X1;
    printf("****  M0X1  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_m1x0.s19");
    op_mode = CPU_MODE_M1X0;
    printf("****  M1X0  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
  
	load_srec("allops_m1x1.s19");
    op_mode = CPU_MODE_M1X1;
    printf("****  M1X1  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_65c02.s19");
    op_mode = CPU_MODE_CMOS_6502;
    printf("****  65c02  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    load_srec("allops_6502.s19");
    op_mode = CPU_MODE_NMOS_6502;
    printf("****  6502  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    disasm(start_address, end_address);
    
    exit(0);
}


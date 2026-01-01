#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"



int disasm_one(uint32_t my_addr, char *outs)
{
    uint32_t val = 0;
    uint8_t op;
    uint8_t oplen;
    address_mode_t addr_mode;
    char param[20];

    outs[0] = (char) 0;
    param[0] = (char) 0;
    op = cpu_read(my_addr);
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
	op_len = get_oplen(cpu_read(lpc));
        for (i = 0; i < 4; i++) {
            if (i < op_len) {
                printf("%02X ", cpu_read(lpc+i));
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



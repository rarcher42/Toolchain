#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "sim.h"
#include "calc_ea.h"

// Riddle:  What's the difference between executing a program and outputting 
// a disassembly?
// Mostly, whether "next instruction" follows the previous instruction (disassembly),
// or follows from the result of executing the previous instruction (execution).
// And whether you print to the user or mutate the CPU state.
//
// This is how we'll unify disassembler and CPU execution unit.
// (No CPUs were harmed in any way in the process)


void get_flags(char *flags)
{
	int i;
	
	for (i = 0; i < 8; i++) {
		flags[i] = '-';
	}
	
	if (GET_FLAG(N_FLAG)) {
		flags[0] = 'N';
	}
	if (GET_FLAG(V_FLAG)) {
		flags[1]  = 'V';
	}
	
	if (GET_FLAG(D_FLAG)) {
		flags[4] = 'D';
	}
	
	if (GET_FLAG(I_FLAG)) {
		flags[5] = 'I';
	}
	
	if (GET_FLAG(Z_FLAG)) {
		flags[6] = 'Z';
	}
	
	if (GET_FLAG(C_FLAG)) {
		flags[7] = 'C';
	}
	
	if ((is_65816()) && (!IS_EMU())) {
		if (GET_FLAG(M_FLAG)) {
			flags[2] = 'M';
		}
		if (GET_FLAG(X_FLAG)) {
			flags[3] = 'X';
		}
	} else {
		if (GET_FLAG(X_FLAG)) {
			flags[3] = 'B';	// Break flag 
		}
	}
}

void dump_registers (void)
{
	char param[32];
	char outs[128];

	if (is_65816()) {
		sprintf(param, "%02X:%04X ", cpu_state.PBR, cpu_state.PC);
		strcpy(outs, param);
	} else {
		sprintf(param, "%04X    ", cpu_state.PC);
	}
	strcpy(outs, param);
	
	if (is_65816()) {
		if (GET_FLAG(M_FLAG)) {
			sprintf(param, "A=%02X B=%02X ", cpu_state.A.AL, cpu_state.A.B);
		} else {
			sprintf(param, "C=%04X    ", cpu_state.A.C);
		}
	} else {
		sprintf(param, "A=%02X      ", cpu_state.A.AL);
	}
	strcat(outs, param);
		
	if ((is_65816()) && (GET_FLAG(X_FLAG) == 0)) {
		sprintf(param, "X=%04X Y=%04X ", cpu_state.X, cpu_state.Y);
	} else {
		sprintf(param, "X=%02X   Y=%02X   ", (cpu_state.X & 0xFF), (cpu_state.Y & 0xFF));
	}
	strcat(outs, param);
	
	if ((!IS_EMU()) && (is_65816())) {
		sprintf(param, "SP=%04X ", cpu_state.SP);
	} else {
		sprintf(param, "SP=%04X ", ((cpu_state.SP & 0xFF) | 0x100));
	}
	strcat(outs, param);
	
	if (is_65816()) {
		sprintf(param, "DPR=%04X ", cpu_state.DPR);
		strcat(outs, param);
		sprintf(param, "DBR=%02X ", cpu_state.DBR);
		strcat(outs, param);
	} else {
		strcat(outs, "                ");
	}
	get_flags(param);
	strcat(outs, param);
	
	printf("%s", outs);
}

#define DEBUG_TEXT_COL_START (36)
void disasm_current (void)
{
    uint32_t addr;
    uint32_t val = 0;
    char param[32];
    char outs[128];
    int i,ls;
    address_mode_t addr_mode;
    uint8_t op;
    uint8_t oplen;

    outs[0] = (char) 0;
    param[0] = (char) 0;
    
    op = get_ir_opcode();
    oplen = get_ir_oplen();
    addr_mode = get_ir_addr_mode();
    addr = get_cpu_address_linear();
    sprintf(outs, "%06X: ", addr);
    for (i = 0; i < 4; i++) {
        if (i < oplen) {
            sprintf(param, "%02X ", cpu_read(addr + i));
            strcat(outs, param);
        } else {
            strcat(outs, "   ");
        }
    } 
    
    val = 0;
    if (oplen > 1) {
        val = from_hex(addr + 1, oplen - 1);
    }   
    sprintf(param, "%s ", get_mnemonic(op));
    strcat(outs, param);
    param[0] = (char) 0;
    switch((int) addr_mode) {
    case OP_NONE:
        break;  // No operands, valid
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
        sprintf(param, "$%04X ", val);
        break;
    
    case OP_REL:
        if (val > 0x7F)
            val = 0x100 - val;
        val = cpu_state.PC + 2 - val;
        sprintf(param, "$%04X ", val);
    break;
    
    case OP_REL_L:
        if (val > 0x7FFF)
            val = 0x10000 - val;
        val = cpu_state.PC + 3 - val;
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
        sprintf(param, "[$%04X] ", val);
        break;

    case OP_ABS_X_IND:
        sprintf(param, "($%04X,X) ", val);
        break;
        
    case OP_STK:
        break;

    default:
        printf("\nUNKNOWN addressing mode %d: aborting!\n", (int) addr_mode);
        exit(0);
    } // switch addr_mode
    strcat(outs, param);
    ls = strlen(outs);
    if (ls < DEBUG_TEXT_COL_START) {
        for (i = 0; i < (DEBUG_TEXT_COL_START - ls); i++) {
            strcat(outs, " ");
        }   // FIXME: inefficient AF but lazy; makes output readable :)
    }
    printf("%s", outs);
    // print_EA();
    dump_registers();
    printf("\n");
    return;
}

// Advance to point to next instruction
// in lexical order.
// Used by disassembler only
static void advance (void)
{
	uint32_t addr;
	
	addr = get_cpu_address_linear() + get_ir_oplen();
	put_cpu_address_linear(addr);	// FIXME: real CPU knows to inc PBR?									
}

void disasm (uint32_t sa, uint32_t ea)
{
    cpu_state.PBR = (sa >> 16) & 0xFF;
    cpu_state.PC = sa & 0xFFFF;
    
    while (get_cpu_address_linear() <= ea) {
        cpu_fetch();  // Next instruction to "execute' (print)
        cpu_decode();
        // We won't execute the instruction in this case :);
        disasm_current();
        advance();  // execute() would ordinarily decide next instr
                    // but we're doing it lexically here
    } 
}



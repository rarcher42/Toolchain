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
    addr = make_linear_address(cpu_state.PBR, cpu_state.PC);
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
    print_EA();
    printf("\n");
    return;
}

// Advance to point to next instruction
// in lexical order.
// Used by disassembler only
static void advance (void)
{
    cpu_state.PC += get_ir_oplen(); 
}

void disasm (uint32_t sa, uint32_t ea)
{
    cpu_state.PBR = (sa >> 16) & 0xFF;
    cpu_state.PC = sa & 0xFFFF;
    
    while ((make_linear_address(cpu_state.PBR, cpu_state.PC)) <= ea) {
        cpu_fetch();  // Next instruction to "execute' (print)
        cpu_decode();
        // We won't execute the instruction in this case :);
        disasm_current();
        advance();  // execute() would ordinarily decide next instr
                    // but we're doing it lexically here
    } 
}



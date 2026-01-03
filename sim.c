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

void change_zflag(uint16_t val, BOOL sixteen)
{
	if (val == 0) {
		SET_FLAG(Z_FLAG);
	} else {
		CLR_FLAG(Z_FLAG);
	}
}

void change_nflag (uint16_t val, BOOL sixteen)
{
	uint16_t msb_mask;
	if (sixteen) {
		msb_mask = 0x8000;
	} else {
		msb_mask = 0x80;
	}
	
	if (val >= msb_mask) {
		SET_FLAG(N_FLAG);
	} else {
		CLR_FLAG(N_FLAG);
	}	
}

void change_vflag (uint16_t in1, uint16_t in2, uint16_t res, BOOL sixteen)
{
	BOOL n1, n2;	// Sign flags of two inputs operands
	BOOL res1;		// Sign flag from result
	uint16_t sign_mask;
	
	if (sixteen) {
		sign_mask = 0x8000;
	} else {
		sign_mask = 0x80;
	}
	n1 = ((in1 & sign_mask) == 1);	// sign bit for in1
	n2 = ((in2 & sign_mask) == 1);	// sign bit for in2
	res1 = ((res & sign_mask) == 1);	// sign bit for result
	if ((n1 == n2) && (res1 != n1)) {
		SET_FLAG(V_FLAG);
	} else {
		CLR_FLAG(V_FLAG);
	}
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
	strcat(outs, param);\
	if (is_65816()) {
		if (IS_EMU()) {
			strcat(outs, " [E]");
		} else {
			strcat(outs, " [N]");
		}
	}
	printf("%s", outs);
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

void load_temp8 (void)
{
	uint32_t dptr;
	uint16_t temp;
	
	dptr = get_EA();
	if (dptr == 0xFFFFFFFF) {
		temp = (uint16_t) get_ir_indexed(1);
		// printf("I8_dynamic_metadata.TEMP = %04X\n", cpu_dynamic_metadata.TEMP);
	} else {
		temp = (uint16_t) cpu_read(dptr);
		// printf("M8 cpu_dynamic_metadata.TEMP = %04X\n", cpu_dynamic_metadata.TEMP);
	}
	cpu_dynamic_metadata.TEMP = temp;
	change_nflag(temp, FALSE);	// All loads and transfers set N and Z flags
	change_zflag(temp, FALSE);
}

void load_temp16 (void)
{
	uint32_t dptr;
	uint16_t temp;
	uint8_t lsb, msb;
	
	dptr = get_EA();
	if (dptr == 0xFFFFFFFF) {
		temp = (get_ir_indexed(2) << 8) | get_ir_indexed(1);
		// printf("I16 cpu_dynamic_metadata.TEMP = %04X\n", cpu_dynamic_metadata.TEMP);	
	} else {
		lsb = cpu_read(dptr);
		msb = cpu_read(dptr + 1);
		temp = (msb << 8) | lsb;
		// printf("M16 cpu_dynamic_metadata.TEMP = %04X\n", cpu_dynamic_metadata.TEMP);
	}
	cpu_dynamic_metadata.TEMP = temp;
	change_nflag(temp, TRUE);	// All load and transfers set N and Z flags
	change_zflag(temp, TRUE);
}

void store_temp16 (void)
{
	uint32_t dptr;
	uint8_t lsb, msb;
	
	dptr = get_EA();
	lsb = cpu_dynamic_metadata.TEMP & 0xFF;
	msb = (cpu_dynamic_metadata.TEMP >> 8) & 0xFF;
	cpu_write(dptr, lsb);
	cpu_write(dptr + 1, msb);
}

void store_temp8 (void)
{
	uint32_t dptr;
	uint8_t lsb;
	
	dptr = get_EA();
	lsb = cpu_dynamic_metadata.TEMP & 0xFF;
	cpu_write(dptr, lsb);
}


void temp_to_A (void)
{
	cpu_state.A.C = cpu_dynamic_metadata.TEMP;	// FIXME: must check all flags
}

void temp_to_X (void)
{
	cpu_state.X = cpu_dynamic_metadata.TEMP;
}

void temp_to_Y (void)
{
	cpu_state.Y = cpu_dynamic_metadata.TEMP;
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
	uint32_t addr;
	void (*fn)(void);
   
	fn = get_op_function(get_ir_opcode());
	(*fn)();
    addr = get_cpu_address_linear() + get_ir_oplen();
    put_cpu_address_linear(addr);   // FIXME: real CPU knows to inc PBR?
        
}

void cpu_run(void)
{
	int kludge = 0;
	
	cpu_dynamic_metadata.running = TRUE;
	while (cpu_dynamic_metadata.running) {
		cpu_fetch();  // Next instruction to "execute' (print)
        cpu_decode();
        // We won't execute the instruction in this case :);
        disasm_current();
        cpu_execute();
        if (++kludge > 1000) {
			cpu_dynamic_metadata.running = FALSE;
		}
    } 
}

int main (void)
{   
    uint32_t start_address;
    uint32_t end_address;
    
    init_vm();  // Create the infrastructure to support memory regions
    alloc_target_system_memory();   // Create the system memory blocks
    print_block_list();

    init_cpu(); 
    
    load_srec("dow.s19", &start_address, &end_address);
    CLR_FLAG(M_FLAG);
    CLR_FLAG(X_FLAG);
    SET_FLAG(N_FLAG);
    SET_FLAG(Z_FLAG);
    set_cpu_type(CPU_65816);
    SET_EMU(FALSE);
    printf("****  EMULATION  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    put_cpu_address_linear(start_address);
    cpu_run();
    // disasm(start_address, end_address);
    exit(0);
}


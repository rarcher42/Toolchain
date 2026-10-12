
// Phase 1:  8 bit 65c02 and 6502 only, with full debugging capability
// but tables set up for full 65c816
// and multiple computer simulation
// Note: Currently, passing cpu_context_t pointers around at all levels.
//
// R. Archer 10/8/26
//
//
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sim_opcodes.h"
#include "sim6502.h"

uint8_t mem_read (virtual_computer_t *computer, uint32_t address)
{
    if (address < computer->memory_size)
        return computer->mem[address];
    else {
        printf("\nInvalid memory address $%08X", address);
        return 0;
    }
}

void mem_write (virtual_computer_t *computer, uint32_t address, uint8_t val)
{
    if (address < computer->memory_size) {
        computer->mem[address] = val;
    } else {
        printf("\nInvalid memory address $%08X", address);
        return;
    }
}

static inline uint8_t hex_val (uint8_t c)
{
    uint8_t g = 0;

    if ((c >= '0') && (c <= '9')) {
        g = c - '0';
    } else if ((c >= 'A') && (c <= 'F')) {
        g = c - '0' - 0x7;
    } else if ((c >= 'a') && (c <= 'f')) {
        g = c - '0' - 0x27;
    }
    return g;
}

uint32_t from_hex_str (uint8_t *s, uint8_t n)
{
    uint32_t sum = 0;
    int i;

    for (i = 0; i < n; i++) {
        sum = (sum << 4) + hex_val(s[i]);
    }
    return sum;
}

void dump_hex (virtual_computer_t *computer, uint32_t sa, uint32_t ea)
{
    uint32_t addr;

    for (addr = sa; addr <= ea; addr++) {
        if ((addr & 0xF) == 0) {
            printf("\r\n%08X: ", addr);
        }
        printf("%02X ", mem_read(computer, addr));
    }
    printf("\r\n");
}

int load_srec (virtual_computer_t *computer, char *fn, uint32_t *start_address, uint32_t *end_address)
{
    int j;
    FILE *fp;
    char rectype;
    char nextline[MAX_LINE];
    uint32_t addr;
    uint8_t nbytes;
    uint8_t b;

    *start_address = 0xFFFFFFFF;
    *end_address = 0;
    addr = 0;
    fp = fopen(fn, "r");
    if (fp != NULL) {
        while (fgets(nextline, MAX_LINE, fp)) {
            ; // printf("%s\r", nextline);
            if (nextline[0] != 'S') {
                return -1;
            }
            rectype = nextline[1];
            switch (rectype) {
                case '0':
                    break;
                case '1':
                    nbytes = from_hex_str((uint8_t *) &nextline[2], 2) - 3;
                    addr = from_hex_str((uint8_t *) &nextline[4], 4);
                    j = 8;
                    while (nbytes-- > 0) {
                        b = from_hex_str((uint8_t *) &nextline[j], 2);
                        mem_write(computer, addr, b);
                        if (addr < *start_address)
                            *start_address = addr;
                        if (addr > *end_address)
                            *end_address = addr;
                        j += 2;
                        addr += 1;
                    }
                    break;
                case '2':
                    nbytes = from_hex_str((uint8_t *) &nextline[2], 2) - 4;
                    addr = from_hex_str((uint8_t *) &nextline[4], 6);
                    j = 10;
                    while (nbytes-- > 0) {
                        b = from_hex_str((uint8_t *) &nextline[j], 2);
                        mem_write(computer, addr, b);
                        if (addr < *start_address)
                            *start_address = addr;
                        if (addr > *end_address)
                            *end_address = addr;
                        j += 2;
                        addr += 1;
                    }
                    break;
                case '3':
                    nbytes = from_hex_str((uint8_t *) &nextline[2], 2) - 5;
                    addr = from_hex_str((uint8_t *) &nextline[4], 8);
                    j = 12;
                    while (nbytes-- > 0) {
                        b = from_hex_str((uint8_t *) &nextline[j], 2);
                        mem_write(computer, addr, b);
                        if (addr < *start_address)
                            *start_address = addr;
                        if (addr > *end_address)
                            *end_address = addr;
                        j += 2;
                        addr += 1;
                    }
                    break;
                case '5':
                    break;
                case '7':
                    break;
                case '8':
                    break;
                case '9':
                    break;
                default:
                    printf("UNKNOWN Record type -  discarding!");
                    break;
            }
        }

    } else {
        return -1;
    }
    dump_hex(computer, *start_address, *end_address);
    return 0;
}


char *get_mnemonic (uint8_t op)
{
    return opcode_table[op].ops;
}

uint8_t fetch_len(uint8_t op)
{
    return opcode_table[op].size & 0x3;    // Ignore M and X flags for now; 8 bit modes
}

void *get_op_function(uint8_t op)
{
    return opcode_table[op].do_op;
}

address_mode_t get_addr_mode (uint8_t op)
{
    return opcode_table[op].adm;
}

void dump_registers (virtual_computer_t  *computer)
{
    uint8_t f;
    char flagstr[9] = "        ";

    f = computer->cpu.reg.P;
    if (f & N_FLAG)
        flagstr[0] = 'N';
    if (f & V_FLAG)
        flagstr[1] = 'V';
    flagstr[2] = '-';
    if (f & B_FLAG)
        flagstr[3] = 'B';
    if (f & D_FLAG)
        flagstr[4] = 'D';
    if (f & I_FLAG)
        flagstr[5] = 'I';
    if (f & Z_FLAG)
        flagstr[6] = 'Z';
    if (f & C_FLAG)
        flagstr[7] = 'C';

    printf("PC=$%04X A=$%02X X=$%02X Y=$%02X SP=$01%02X P=%s\n", computer->cpu.reg.PC, computer->cpu.reg.A, computer->cpu.reg.X, computer->cpu.reg.Y, computer->cpu.reg.S, flagstr);
}

void disasm (virtual_computer_t *computer, uint32_t start, uint32_t end)
{
    uint32_t vpc;
    uint8_t fetchlen;
    int i;

    vpc = start;
    while (vpc <= end) {
        fetchlen = fetch_len(computer->mem[vpc]);
        printf("%s  ", get_mnemonic(computer->mem[vpc]));
        for (i = 0; i < fetchlen; i++) {
            printf(" %02X", computer->mem[vpc+i]);
        }
        printf("\n");
        vpc += fetchlen;
    }
}

void print_ir (virtual_computer_t *computer)
{
    int i;

    printf("$%04X: ", computer->cpu.reg.PC);
    for (i = 0; i < computer->cpu.meta.oplen; i++) {
        printf("%02X ", computer->cpu.meta.ir[i]);
    }
    printf("\n");
}

void fetch (virtual_computer_t *computer)
{
    int i;
    uint16_t ip;
    uint8_t opcode;

    ip = computer->cpu.reg.PC;
    opcode = computer->mem[ip];
    computer->cpu.meta.oplen = fetch_len(opcode);
    for (i = 0; i < computer->cpu.meta.oplen; i++) {
        computer->cpu.meta.ir[i] = computer->mem[ip + i];
    }
    print_ir(computer);
}

void decode (virtual_computer_t *computer)
{

}

void execute (virtual_computer_t *computer)
{
    opcode_table[computer->cpu.meta.ir[0]].do_op();
    computer->cpu.reg.PC += computer->cpu.meta.oplen;
}

// Create another virtual computer.  Add I/O, memory access rights, etc. along the way
virtual_computer_t *create_computer(cpu_type_t cpu_type, uint32_t memory_size)
{
    uint8_t *mem_ptr;
    virtual_computer_t *computer = NULL;

    mem_ptr = (uint8_t *) malloc(memory_size);
    if (mem_ptr) {
        computer = (virtual_computer_t *) malloc(sizeof(virtual_computer_t));
        if (computer) {
            computer->memory_size = memory_size;
            computer->mem = mem_ptr;
            computer->cpu.meta.cpu_type = cpu_type;
        }
    }
    return computer;
}

int main (void)
{
    uint32_t start_address;
    uint32_t end_address;
    int i;
    virtual_computer_t *computer;

    computer = create_computer(M6502, MEMORY_SIZE);
    load_srec(computer, "dow.s28", &start_address, &end_address);
    printf("\nStart address: $%08X", start_address);
    printf("\n  End address: $%08X\n", end_address);
    computer->cpu.reg.PC = start_address;
    dump_registers(computer);
    // disasm(computer, start_address, end_address);
    for (i = 0; i < 30; i++) {
        fetch(computer);
        decode(computer);
        execute(computer);
    }
    printf("\n\n\nCPU simulator done!\n");
}

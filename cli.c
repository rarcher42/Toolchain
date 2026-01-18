#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "optbl_65816.h"
#include "vm.h"
#include "srec.h"
#include "disasm.h"
#include "cpu65xx.h"
#include "breakpoint.h"
#include "calc_ea.h"
#include "opcodes.h"



#define MAX_CMD_LINE (132)

struct termios orig_termios;

void disableRawMode (void) 
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode (void) 
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    raw.c_oflag &= ~OPOST;
    raw.c_cflag |= (CS8);
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void init_raw_io (void)
{
	// Disable C library buffering for stdin and stdout
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    // Enable raw terminal mode
    enableRawMode();
}

void get_line(char *line)
{
	int i;
	BOOL editing;
	char c;
	
	i = 0;
	editing = TRUE;
	
	while ((editing) && (i < MAX_CMD_LINE-1)) {
		read(STDIN_FILENO, &c, 1);
		// printf("%d ", c);
		
		if ((c == 127) || (c == 8)) {
			if (i > 0) {
				line[i--] = 0;
				printf("%c%c%c", 8, 32, 8);
			}
		} else if (c == 3) {
			line[0] = 0;
			printf("^C");
			editing = FALSE;
		} else if (c == 13) {
			line[i++] = 0;
			printf("%c%c", 13,10);
			editing = FALSE;
		} else {
			line[i++] = c;
			printf("%c", c);
		}
	}
	if (editing) {
		printf("\n\rLine too long at %d - discarded", i);
		line[0] = 0;
	}
}

void get_cmd(char *line)
{
	printf(">");
	get_line(line);
}


void upcase(char *lp)
{
	char c;
	
	while ((c = *lp) != 0) {
		if ((c >= 'a') && (c <= 'z')) {
			*lp = c - ('a' - 'A');
		}
		++lp;
	}
}

// Starting at character position pos, return everything
// up to the next space or CR or NULL, and return a NULL
// in their place in tok, return next position or 0 if end of line
// reached.  This is sum bare metal shizzle.
int get_next_arg (char *tok, char *line, int pos)
{
	int i, j;
	char c;
		
	i = pos;
	j = 0;
	
	// Skip leading spaces in field
	while ((c = line[i]) == ' ')
		i++;
	// Look for trailing space or EOL
	while (((c = line[i]) != ' ') && (c != 0)) {
		tok[j++] = c;
		i++;
	}
	tok[j] = 0;
	if (c != 0)
		return i;
	
	return 0;
}


BOOL process_cmd (char *line)
{
	int pos;
	char tok[MAX_CMD_LINE];
	uint32_t sa, ea;
	
	printf("process_cmd(%s)\r\n", line);
	// Get parameter #0 (the command verb) & upper case it
	pos = 0;
	pos = get_next_arg(tok, line, pos);
	upcase(tok);
	// printf("CMD(%s)%d\r\n", tok, pos);
	
	if (strncmp(tok, "LOAD", 4) == 0) {
		pos = get_next_arg(tok, line, pos);
		// printf("\r\nFN=%s\r\n", tok);
		printf("LOADING %s...\r\n", tok);
		load_srec(tok, &sa, &ea);
		if (sa == 0xFFFFFFFF) {
			printf("Error loading %s\r\n", tok);
		} else {
			printf("Loaded memory $%06X-$%06X\r\n", sa, ea);
		}
		
	} else if (strncmp(tok, "Q", 1) == 0) {
		printf("\r\nQuitten\r\n");
		return FALSE;
	}
	return TRUE;
}

int main (void)
{   
    uint32_t start_address;
    uint32_t end_address;
    struct timespec start, end;
    long long elapsed_nanoseconds;
    int i;
    BOOL parsing;
    char line[MAX_CMD_LINE];
	
	init_raw_io();
    init_vm();  // Create the infrastructure to support memory regions
    alloc_target_system_memory();   // Create the system memory blocks
    init_breakpoints();
    // print_block_list();
    
    init_cpu(); 
    SET_FLAG(M_FLAG);
    SET_FLAG(X_FLAG);
    CLR_FLAG(D_FLAG);
    CLR_FLAG(I_FLAG);
    SET_EMU(TRUE);
    
    set_cpu_type(CPU_6502);
  
	do {
		get_cmd(line);
		parsing = process_cmd(line);
	} while (parsing == TRUE);
	
	exit(0);
    load_srec("validate_6502.s19", &start_address, &end_address);

    printf("****  sa = %08X, ea=%08X ***** \n", start_address, end_address);
    put_cpu_address_linear(start_address);
    // Get the start time
    clock_gettime(CLOCK_MONOTONIC, &start);
    set_print_execution(TRUE);		// Suppress or allow printing of info during execution
    add_breakpoint(0x21C0, BP_EX);
    add_breakpoint(0x21C3, BP_EX);
	cpu_run(TRUE);
	for (i=0; i < 256; i++) 
		cpu_resume();
    // Get the end time
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate the difference in microseconds
    elapsed_nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000LL +
                           (end.tv_nsec - start.tv_nsec);

    printf("\n\nElapsed time: %lld microseconds\n", elapsed_nanoseconds / 1000L);
    // disasm(start_address, end_address);
    printf("\n%ld instructions executed\n\n", get_cpu_instruction_count());
}


#if 0
int main() 
{
	BOOL parsing;
    char line[MAX_CMD_LINE];
	
	init_raw_io();
	
	do {
		get_cmd(line);
		parsing = process_cmd(line);
	} while (parsing == TRUE);
	
    return 0;
}
#endif

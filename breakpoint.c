#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "optbl_65816.h"
#include "sim.h"
#include "breakpoint.h"



breakpoint_t bp_table[MAX_BREAKPOINTS];
uint8_t bp_count = 0;	// Revisit if MAX_BREAKPOINTS increases



// Note, a linear search of the entire array is an initial
// implementation.  So many features to add, so we'll just
// do this and FIXME later
void init_breakpoints (void)
{
	int i;
	
	for (i = 0; i < MAX_BREAKPOINTS; i++) {
		bp_table[i].address = NULL_PTR;	// Not valid > 24 bits
		bp_table[i].access = 0;		// No permissions set
	}	
	bp_count = 0;
}

// FIXME: probably should sort entries so search can stop sooner,
// move allocated blocks to the front of the list without fragmented
// erased breakpoints to wade through.  For now, we have to search
// the entire list.
BOOL add_breakpoint (uint32_t address, uint8_t perms)
{
	uint32_t addr;
	int i;
	
	for (i = 0; i < MAX_BREAKPOINTS; i++) {
		addr = bp_table[i].address;
		if (addr == NULL_PTR) {
			// Encountered a free block
			bp_table[i].address = address;
			bp_table[i].access = perms;
			++bp_count;
			return FALSE;
		} else if (addr == address) {
			bp_table[i].access = perms;		// Overwrite permissions
			++bp_count;
			return FALSE;
		}
	}
	return TRUE;	// No breakpoint was set (probably table is full
}

static void consolidate_breakpoint (void)
{
	int hi, empty;
	
	if (bp_count > 0) {
		for (empty = 0; empty < MAX_BREAKPOINTS; empty++) {
			if (bp_table[empty].address == NULL_PTR) {
				for (hi = empty+1; hi < MAX_BREAKPOINTS; hi++) {
					if (bp_table[hi].address != NULL_PTR) {
						// Move it down to earliest empty slot
						bp_table[empty].address = bp_table[hi].address;
						bp_table[empty].access = bp_table[hi].access;
						bp_table[hi].address = NULL_PTR;
						bp_table[hi].access = 0;
					}
				}
			}
		}
	}
}

uint8_t is_breakpoint (uint32_t address)
{
	int i;
	
	for (i = 0; i < bp_count; i++) {
		if (bp_table[i].address == address) 
			return bp_table[i].access;	// Return RWX flags for breakpt.
	}
	return 0;	// No match <==> no breakpoint case
}

void print_bp (uint32_t address)
{
	uint8_t perms;
	
	perms = is_breakpoint(address);
	printf("Address: $%08X rwx=%d\n", address, perms);
}

BOOL delete_breakpoint (uint32_t address)
{
	int i;
	
	for (i = 0; i < MAX_BREAKPOINTS; i++) {
		if (bp_table[i].address == address) {
			bp_table[i].address = NULL_PTR;
			bp_table[i].access = 0;
			--bp_count;
			consolidate_breakpoint();
			return FALSE;
		}
	}
	return TRUE;
}

void print_breakpoints(void)
{
	int i, j;
	uint32_t address;
	uint8_t perms;
	
	if (bp_count > 0) {
		printf("==== Breakpoints ====\n");
		j = 0;
		for (i = 0; i < MAX_BREAKPOINTS; i++) {
			address = bp_table[i].address;
			if (address != NULL_PTR) {
				printf("%02d: $%06X    ", j++, address);
				perms = bp_table[i].access;
				if (perms & BP_RD) {
					printf("R");
				} 
				if (perms & BP_WR) {
					printf("W");
				}
				if (perms & BP_EX) {
					printf("X");
				}
				printf("\n");
			}
		} // for
	} else {
		printf("\nNo breakpoints set\n");
	}
}


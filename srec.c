#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "disasm.h"
#include "vm.h"

const int MAX_LINE = 130;


static inline uint8_t hex_val(uint8_t c)
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

uint32_t from_hex_str(uint8_t *s, uint8_t n)
{
    uint32_t sum = 0;
    int i;

    for (i = 0; i < n; i++) {
        sum = 16*sum + hex_val(s[i]);
    }
    return sum;
}

uint32_t from_hex(uint32_t addr, uint8_t n)
{
    uint32_t sum = 0;
    int i;
 
    for (i = n-1; i >= 0; --i) {
        // printf("_%02X_", numseq[i]);
        sum = 256*sum + read_byte(addr+i);
    }
    return sum;
}

void dump_hex(uint32_t sa, uint32_t ea)
{
    uint32_t addr;

    for (addr = sa; addr <= ea; addr++) {
        if ((addr & 0xF) == 0) {
            printf("\n%08X: ", addr);
	} 
	printf("%02X ", read_byte(addr));
    }
    printf("\n");
}


int load_srec(char *fn)
{
    int j;
    FILE *fp;
    char rectype;
    char nextline[MAX_LINE];
    uint32_t addr;
    uint8_t nbytes;
    uint8_t b;

    start_address = 0xFFFFFFFF;
    end_address = 0;
    addr = 0;
    fp = fopen(fn, "r");
    if (fp != NULL) {
        while (fgets(nextline, MAX_LINE, fp)) {
		    puts(nextline);
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
				write_byte(addr, b);
				if (addr < start_address)
	               start_address = addr; 
				if (addr > end_address)
					end_address = addr;
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
                    write_byte(addr, b);
                    if (addr < start_address)
                       start_address = addr;
                    if (addr > end_address)
                        end_address = addr;
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
                    write_byte(addr, b);
					if (addr < start_address)
                       start_address = addr;
                    if (addr > end_address)
                        end_address = addr;
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
    dump_hex(start_address, end_address);
    return 0;
}

#define MAX_BREAKPOINTS (32)

#define BP_RD	(0x4)
#define BP_WR	(0x2)
#define BP_EX  	(0x1)

typedef struct {
	uint32_t address;		// Breakpoint address or 0xFFFF FFFF if empty
	uint8_t access;			// 00000rwx
} breakpoint_t;


void init_breakpoints(void);
BOOL add_breakpoint(uint32_t address, uint8_t perms);
uint8_t is_breakpoint (uint32_t address);
void print_bp(uint32_t address);
BOOL delete_breakpoint(uint32_t address);
void print_breakpoints(void);

#define MEMORY_SIZE    (65536)


#define N_FLAG 0x80
#define V_FLAG 0x40
#define B_FLAG 0x10
#define D_FLAG 0x08
#define I_FLAG 0x04
#define Z_FLAG 0x02
#define C_FLAG 0x01

typedef enum {M6502=1, M65C02=2, M65C816=4} cpu_type_t;

typedef struct
{
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S;
    uint16_t PC;
    uint8_t P;
} cpu_reg_t;

typedef struct cpu_meta
{
    uint8_t oplen;
    uint8_t ir[4];
    uint32_t cycles;
    cpu_type_t cpu_type;
    void *virtual;    // So CPU can address associated memory once virtual computer created
} cpu_metadata_t;

typedef struct
{
    cpu_reg_t reg;
    cpu_metadata_t meta;
} cpu_context_t;

typedef struct {
    cpu_context_t cpu;
    uint32_t memory_size;
    uint8_t *mem;
} virtual_computer_t;


// CPU op table stuff
#define MAX_OPSTR_LEN (3)


typedef enum
{
    OP_NONE = 0, OP_A, OP_IMM, OP_ABS, OP_ABS_L, OP_ABS_IND, OP_ABS_IND_L,
    OP_ABS_X, OP_ABS_Y, OP_ABS_X_L, OP_ABS_X_IND, OP_ZP,
    OP_ZP_IND, OP_ZP_IND_L, OP_ZP_X, OP_ZP_Y, OP_ZP_XI,
    OP_ZP_IY, OP_ZP_IY_L, OP_REL, OP_REL_L, OP_SR, OP_SR_IY,
    OP_STK
} address_mode_t;

typedef struct op_tbl
{
    char ops[MAX_OPSTR_LEN+1];    // Hold full opcode plus null
    uint16_t support_flags;      // Flags for unsupported CPU versions, from cpu_type_t enum
    uint8_t size;
    address_mode_t adm;
    void (*do_op)(cpu_context_t *cpu);	// Do the opcode!

} op_tbl;


char *get_mnemonic(uint8_t op);
uint8_t fetch_len(uint8_t op);
uint32_t get_EA(cpu_context_t *cpu);
void set_flag(cpu_context_t *cpu, uint8_t flag);
void clr_flag(cpu_context_t *cpu, uint8_t flag);
uint8_t get_flag(cpu_context_t *cpu, uint8_t flag);
uint8_t cpu_read(cpu_context_t *cpu, uint32_t address);
void cpu_write(cpu_context_t *cpu, uint32_t address, uint8_t val);


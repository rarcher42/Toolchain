

#define MAX_OPS_LEN (3)
typedef enum
{
    OP_NONE = 0, OP_A, OP_IMM, OP_ABS, OP_ABS_L, OP_ABS_IND, OP_ABS_IND_L,
    OP_ABS_X, OP_ABS_Y, OP_ABS_X_L, OP_ABS_X_IND, OP_ZP,
    OP_ZP_IND, OP_ZP_IND_L, OP_ZP_X, OP_ZP_Y, OP_ZP_XI,
    OP_ZP_IY, OP_ZP_IY_L, OP_REL, OP_REL_L, OP_SR, OP_SR_IY,OP_2OPS, 
    OP_STACK
} address_mode_t;

typedef struct op_tbl
{
    char ops[MAX_OPS_LEN+1];    // Hold full opcode plus null
    uint8_t unsupport;		// Flags for unsupported CPU versions
    uint8_t sizeinfo;
    address_mode_t adm;

} op_tbl;

extern op_tbl opcode_table[];
extern const uint8_t LEN1;
extern const uint8_t LEN2;
extern const uint8_t LEN3;
extern const uint8_t LEN4;
extern const uint8_t M_ADDS;        // If M flag is 0, add 1 to ilen
extern const uint8_t X_ADDS;        // If X = 0, then add 1 to inst len
extern const uint8_t NOT_65C02;     // Instruction not supported 65c02
extern const uint8_t NOT_6502;      // Instruction not supported NMOS

extern address_mode_t get_addr_mode(uint8_t op);
uint8_t get_oplen(uint8_t op);

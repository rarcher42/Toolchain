

typedef uint8_t BOOL;
#define FALSE (0)
#define TRUE (!FALSE)


#define N_FLAG 0x80
#define V_FLAG 0x40
#define M_FLAG 0x20
#define X_FLAG 0x10
#define D_FLAG 0x08
#define I_FLAG 0x04
#define Z_FLAG 0x02
#define C_FLAG 0x01

#define CPU_65816 (0)
#define CPU_6502 (1)
#define CPU_65c02 (2)


typedef union {
    uint16_t C;
    struct {
        uint8_t AL;
        uint8_t B;
    };
} a_reg_t;

// Actual CPU registers as seen by programmer
typedef struct {
    a_reg_t A;
    uint16_t X;
    uint16_t Y;
    uint16_t DPR;
    uint16_t SP;
    uint16_t PC;
    uint8_t DBR;
    uint8_t PBR;
    uint8_t flags;
    uint8_t em;
} cpu_state_t;


// CPU metadata here.  This is stuff that might be hardware in the CPU,
// and in any event are very convenient to track CPU behavior with.
typedef struct {

    uint8_t oplen;              // Remember operation length
    address_mode_t  addr_mode;  // Remember address mode
    uint8_t ir[4];              // Virtual instruction
                                // register 0..oplen-1 valid
} cpu_dynamic_metadata_t;

// This is data that would never change at run-time, like what
// CPU we're emulating
typedef struct {
    // cpu_type excludes unimplemented instructions, including
    // maybe that 65c816 in E mode runs instructions the 65c02 can't
    // (FIXME: verify) so one program can handle all the major
    // 6502 family variants
    // FIXME: Rockwell bit instructions, 65E02, 6510, etc.?
    //
    uint8_t cpu_type;           // 0 = 65816
                                // 1 = 6502
                                // 2 = 65c02
} cpu_static_metadata_t;

// Track events inside or entering the CPU
typedef struct {
    uint8_t nmi_pending;        // NMI made a new high->low transition
    uint8_t irq_pending;        // IRQ is low (active)
    uint8_t reset_pending;      // RESET signal went low
} cpu_event_metadata_t;


extern void SET_FLAG (uint8_t fset_mask);
extern void CLR_FLAG (uint8_t fres_mask);
extern uint8_t GET_FLAGS(void);
extern uint8_t GET_FLAG(uint8_t flag);
extern uint8_t get_cpu_type(void);
extern char *get_mnemonic(uint8_t op);
extern uint32_t calc_EA(void);
// The cpu_fetch() function populates the instruction register
// and metadata as sharable metadata across modules.
extern void cpu_fetch(uint32_t addr);
uint8_t get_ir_opcode(void);	// The op-code just fetched
extern uint8_t get_ir_oplen(void);	// The op-code length
extern address_mode_t  get_ir_addr_mode(void);	// OP's address mode
extern uint8_t get_ir_indexed(uint8_t index);	// Get any ir reg byte


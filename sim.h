

typedef uint8_t BOOL;
#define FALSE (0)
#define TRUE (!FALSE)


#define N_FLAG 0x80
#define V_FLAG 0x40
#define M_FLAG 0x20
#define X_FLAG 0x10
#define B_FLAG 0x10
#define D_FLAG 0x08
#define I_FLAG 0x04
#define Z_FLAG 0x02
#define C_FLAG 0x01

#define CPU_65816 (0)
#define CPU_6502 (1)
#define CPU_65C02 (2)


typedef union {
    uint16_t C;
    struct {
        uint8_t AL;
        uint8_t B;
    };
} a_reg_t;

// Actual CPU registers visible to programmer
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
    BOOL running;               // CPU is running if TRUE, halted otherwise
    uint32_t cycle_counter;     // Cycle counter
    unsigned long instruction_counter;
    uint8_t oplen;              // Remember operation length
    uint16_t fetch_pc;          // FIXME: PC at fetch time needed for integration for now
    uint8_t fetch_pbr;          // FIXME: PBR at fetch time; remove after integration
    address_mode_t  addr_mode;  // Remember address mode
    uint32_t EA;                // calculated effective address
    uint16_t TEMP;              
    // uint8_t TEMP_L;
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
extern uint8_t IS_EMU(void);
extern void SET_EMU(uint8_t em);
extern uint8_t get_cpu_type(void);
extern BOOL is_65816(void);
BOOL is_6502(void);
BOOL is_65C02(void);
extern void change_vflag(uint16_t in1, uint16_t in2, uint16_t res, BOOL sixteen);
extern uint16_t get_dpr(void);
extern uint8_t get_dbr(void);
extern uint8_t get_pbr(void);
extern void get_flags(char *flags);
extern void dump_registers(void);
// For tightly-coupled helper modules (e.g. calc_ea), 
// allow acess to cpu_state.  Use accessor functions later?
extern cpu_state_t cpu_state;
extern cpu_dynamic_metadata_t cpu_dynamic_metadata;

// Various accessor functions
extern char *get_mnemonic(uint8_t op);
// The cpu_fetch() function populates the instruction register
// and metadata as sharable metadata across modules.
uint8_t get_ir_opcode(void);    // The op-code just fetched
extern uint8_t get_ir_oplen(void);  // The op-code length
extern address_mode_t  get_ir_addr_mode(void);  // OP's address mode
extern uint8_t get_ir_indexed(uint8_t index);   // Get any ir reg byte

extern void set_EA(uint32_t ea);
extern uint32_t get_EA (void);
extern uint32_t make_linear_address(uint8_t bank, uint16_t pc);
extern uint32_t get_cpu_address_linear(void);
extern void put_cpu_address_linear(uint32_t address);
extern void load_temp16 (void);
extern void load_temp8(void);
void store_temp16(void);
void store_temp8(void);
extern void cpu_fetch(void);
extern void cpu_decode(void);


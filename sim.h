

typedef uint8_t BOOL;
#define FALSE (0)
#define TRUE (!FALSE)

typedef union {
    uint16_t AX;
    struct {
        uint8_t AL;
        uint8_t B;
    };
} a_reg_t;

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

// Hold CPU metadata here
typedef struct {
    uint8_t cpu_type;           // 0 = 65816
                                // 1 = 6502
                                // 2 = 65c02
    uint8_t oplen;              // Remember operation length
    address_mode_t  addr_mode;  // Remember address mode
    uint8_t ir[4];              // Virtual instruction register 0..oplen-1
} meta_cpu_state_t;

extern const uint8_t N_FLAG;
extern const uint8_t V_FLAG;
extern const uint8_t M_FLAG;
extern const uint8_t X_FLAG;
extern const uint8_t D_FLAG;
extern const uint8_t I_FLAG;
extern const uint8_t Z_FLAG;
extern const uint8_t C_FLAG;

extern cpu_state_t cpu_state;
extern meta_cpu_state_t meta_cpu_state;


extern void SET_FLAG (uint8_t fset_mask);
extern void CLR_FLAG (uint8_t fres_mask);
extern uint8_t GET_FLAGS(void);
extern uint8_t GET_FLAG(uint8_t flag);
extern uint8_t get_cpu_type(void);
extern uint32_t calc_EA(void);
extern void cpu_fetch(uint32_t addr);

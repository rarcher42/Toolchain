

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



extern cpu_state_t cpu_state;
extern void SET_FLAG (uint8_t fset_mask);
extern void CLR_FLAG (uint8_t fres_mask);
extern uint8_t GET_FLAGS(void);
extern uint8_t GET_FLAG(uint8_t flag);
extern uint8_t get_cpu_type(void);
extern uint32_t calc_EA(uint8_t op);
extern const uint8_t N_FLAG;
extern const uint8_t V_FLAG;
extern const uint8_t M_FLAG;
extern const uint8_t X_FLAG;
extern const uint8_t D_FLAG;
extern const uint8_t I_FLAG;
extern const uint8_t Z_FLAG;
extern const uint8_t C_FLAG;

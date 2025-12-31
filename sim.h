

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


const uint8_t FLAG_N = (1 << 7);
const uint8_t FLAG_V = (1 << 6);
const uint8_t FLAG_M = (1 << 5);
const uint8_t FLAG_X = (1 << 4);
const uint8_t FLAG_D = (1 << 3);
const uint8_t FLAG_I = (1 << 2);
const uint8_t FLAG_Z = (1 << 1);
const uint8_t FLAG_C = 0x01;



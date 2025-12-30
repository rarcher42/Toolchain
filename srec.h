extern int load_srec(char *fn);
extern void dump_hex(uint32_t sa, uint32_t ea);
extern uint32_t from_hex_str(uint8_t *s, uint8_t n);
extern uint32_t from_hex(uint32_t addr, uint8_t n);

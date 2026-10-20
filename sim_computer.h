void create_computer(cpu_type_t cpu_type);
uint8_t mem_read(uint32_t address);
void mem_write(uint32_t address, uint8_t val);
void dump_hex(uint32_t sa, uint32_t ea);
int load_srec(char *fn, uint32_t *start_address, uint32_t *end_address);
void disasm(uint32_t start, uint32_t end);

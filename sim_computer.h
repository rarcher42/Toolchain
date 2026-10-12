virtual_computer_t *create_computer(cpu_type_t cpu_type, uint32_t memory_size);
uint8_t mem_read(virtual_computer_t *computer, uint32_t address);
void mem_write(virtual_computer_t *computer, uint32_t address, uint8_t val);
void dump_hex(virtual_computer_t *computer, uint32_t sa, uint32_t ea);
int load_srec(virtual_computer_t *computer, char *fn, uint32_t *start_address, uint32_t *end_address);
void disasm(virtual_computer_t *computer, uint32_t start, uint32_t end);

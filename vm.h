
typedef struct {
    uint8_t *mem;
    uint32_t saddr;
    uint32_t eaddr;
    // Each device has an implementation FSM / code that understands
    // the device's characteristics.  For example, read/write vs.
    // read only, block write (FLASH), and peripheral states
    // implmentation(address, data, wr)
    // (For wr=1, data = data to write
    // for wr=0, data is don't care)
    int (*implementation)(void *self, uint32_t, uint8_t, uint8_t);
    void *next;
} mem_block_descriptor_t;

extern void init_mem(void);
extern int alloc_block(uint32_t saddr, uint32_t eaddr,
    int (*handler)(void *self, uint32_t addr, uint8_t data, 
		uint8_t wr));
extern int del_block_containing(uint32_t ma);
extern mem_block_descriptor_t *find_block_descriptor(uint32_t ma);
extern void print_block_list(void);
extern int write_byte(uint32_t addr, uint8_t data);
extern int read_byte(uint32_t addr);

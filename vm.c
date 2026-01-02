#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "vm.h"


mem_block_descriptor_t *mem_list_head;

void init_vm(void)
{
    mem_list_head = NULL;
}

int alloc_block(uint32_t saddr, uint32_t eaddr,
    int (*handler)(void *self, uint32_t addr, uint8_t data, uint8_t wr))
{
    uint32_t bl;
    uint8_t *mp;    // Allocated memory block
    mem_block_descriptor_t *mbd;

    if (eaddr < saddr) {
        bl = saddr;
        saddr = eaddr;
        eaddr = bl;
    }
    bl = eaddr - saddr + 1;
    mp = malloc(bl);
    if (mp == NULL)
        return -1;
    mbd = malloc(sizeof(mem_block_descriptor_t));
    if (mbd == NULL) {
        free(mp);
        return -1;
    }
    mbd->mem = mp;
    mbd->saddr = saddr;
    mbd->eaddr = eaddr;
    mbd->implementation = handler;
    mbd->next = mem_list_head;
    mem_list_head = mbd;
    return 0;    
}

int del_block_containing(uint32_t ma)
{
    mem_block_descriptor_t *mbp;
    mem_block_descriptor_t *prev;

    mbp = mem_list_head;
    prev = NULL;
    while (mbp != NULL) {
        if ((ma >= mbp->saddr) && (ma <= mbp->eaddr)) {
        printf("Found it!  0x%08X-0x%08X", mbp->saddr, mbp->eaddr);
        if (prev == NULL) {
                // Delete first item on the list
            mem_list_head = mbp->next;
            free(mbp->mem);     // Free actual memory
            free(mbp);      // Free descriptor
        } else {
            prev->next = mbp->next; // Link across deleted element
        free(mbp->mem);
        free(mbp);
        }
    }
    prev = mbp;
    mbp = mbp->next;
    }
    return 0;
}


mem_block_descriptor_t *find_block_descriptor(uint32_t ma)
{

    mem_block_descriptor_t *mbp;

    mbp = mem_list_head;
    while (mbp != NULL) {
    if ((ma >= mbp->saddr) && (ma <= mbp->eaddr)) {
        return mbp;
        }
        mbp = mbp->next;
    }
    return (mem_block_descriptor_t *) NULL;
}


void print_block_list(void)
{
    mem_block_descriptor_t *mbp;

    mbp = mem_list_head;
    while (mbp != NULL) {
        printf("====================================\n");
        printf("Self:        : *%p\n",  mbp);
        printf("Start address: $%08X\n", mbp->saddr);
        printf("End   address: $%08X\n", mbp->eaddr);
        printf("Next         : *%p\n\n", mbp->next);    
        mbp = mbp->next;
    }
}

int cpu_write(uint32_t addr, uint8_t data)
{
    mem_block_descriptor_t *bdp;
    // Implement caching!  For now, just make it work
    bdp = find_block_descriptor(addr);
    if (bdp == NULL) {
        printf("LOCATION %08X not found!\n", addr);
        return -1;
    }
    return bdp->implementation((void *) bdp, addr, data, 1);    // Do write via handler
}

int cpu_read(uint32_t addr)
{
    mem_block_descriptor_t *bdp;
    uint8_t val;
    // Implement caching!  For now, just make it work
    bdp = find_block_descriptor(addr);
    if (bdp == NULL) {
        printf("LOCATION %08X not found!\n", addr);
        return -1;
    }
    val = bdp->implementation((void *) bdp, addr, 0, 0);    // Do read via handler
    return val;
}

int handler_via1 (void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    printf("Warning: no handler for VIA: called handler_via1($%08X, $%02X. wr=%d)\n", addr, data, wr);
    return 0;
}

int handler_via2 (void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    printf("cWarning: no handler for VIA2: called handler_via2($%08X, $%02X), wr=%d\n", addr, data, wr);
    return 0;
}

int handler_acia(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    printf("cWarning: no handlier for ACIA: alled handler_acia($%08X, $%02X), wr=%d\n", addr, data, wr);
    return 0;
}

int handler_pia(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    printf("Warning: No hanlder for PIA: called handler_pia($%08X, $%02X), wr=%d\n", addr, data, wr);
    return 0;
}

int handler_io_unimplemented(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    printf("Unhandled I/O device: called handler_io_unimplemented($%08X, $%02X)\n, wr=%d", addr, data, wr);
    return 0;
}


int handler_null(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    return 0;
}

int handler_ram(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
    mem_block_descriptor_t *p;
    
    p = (mem_block_descriptor_t *) bdp;
    if (p == NULL) {
        printf("\nCannot locate RAM descriptor: R/W operation failed!\n");
        return -1;
    }
    if (wr) {
        //cprintf("Writing $%02X to RAM location %08X\n", data, addr);
        p->mem[addr - p->saddr] = data;
        return 0;
    } else {
        // printf("Reading from RAM location %08X\n", addr);
        return p->mem[addr - p->saddr];
    }
}

int handler_rom(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{

    mem_block_descriptor_t *p;
    
    p = (mem_block_descriptor_t *) bdp;
    if (p == NULL) {
        printf("\nCannot locate ROM descriptor: R/W operation failed!\n");
        return -1;
    }
    if (wr) {
        printf("Error:  cannot write to ROM location $%08X\n", addr);
        return 0;
    } else {
        // printf("Reading from ROM location $%08X\n", addr);
        return p->mem[addr - p->saddr];
    }
}


int handler_flash(void *bdp, uint32_t addr, uint8_t data, uint8_t wr)
{
        mem_block_descriptor_t *p;
    
    p = (mem_block_descriptor_t *) bdp;
    if (p == NULL) {
        printf("\nCannot locate ROM descriptor: R/W operation failed!\n");
        return -1;
    }
    if (wr) {
        // TODO: could implement simulate flash behavior here
        printf("Error:  cannot write to ROM location $%08X\n", addr);
        return 0;
    } else {
        // printf("Reading from ROM location $%08X\n", addr);
        return p->mem[addr - p->saddr];
    }
}


void alloc_target_system_memory(void) 
{
   // Set up the memory regions for the target system
    alloc_block(0x7F00, 0x7F1F, handler_io_unimplemented);  // XBUS0 (not implmemented)
    alloc_block(0x7F20, 0x7F3F, handler_io_unimplemented);  // XBUS1 (not implemented)
    alloc_block(0x7F40, 0x7F5F, handler_io_unimplemented);  // XBUS2 (not implemented)
    alloc_block(0x7F60, 0x7F7F, handler_io_unimplemented);  // XBUS3 (not implemented)
    alloc_block(0x7F80, 0x7F9F, handler_acia);  // ACIA
    alloc_block(0x7FA0, 0x7FBF, handler_pia);   // PIA
    alloc_block(0x7FC0, 0x7FDF, handler_via1);  // VIA
    alloc_block(0x7FE0, 0x7FFF, handler_via2);  // USB VIA
    alloc_block(0x0, 0x7EFF, handler_ram);      // RAM
    alloc_block(0x8000, 0xFFFF, handler_ram); // FLASH - changed to RAM for vector testing
}

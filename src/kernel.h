#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define VIOS_MAX_PATH 108

struct paging_4gb_chunk; // Forward declaration
extern struct paging_4gb_chunk *kernel_chunk;

// Kernel entry point
void kernel_main(void);

// Enables kernel paging and switches to the kernel page directory
void kernel_page(void);

// Registers kernel registers or setups CPU state (assumed)
void kernel_registers(void);

// Enable interrupts
void enable_interrupts(void);

// Error handling macros
#define ERROR(value) ((void *)(value))
#define ERROR_I(value) ((int)(value))
#define ISERR(value) ((int)(value) < 0)

#endif // KERNEL_H

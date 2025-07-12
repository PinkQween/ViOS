#ifndef KERNEL_INIT_H
#define KERNEL_INIT_H

#include <stdint.h>
#include <stdbool.h>

struct mouse;

/**
 * Initialize the Global Descriptor Table and Task State Segment
 */
void kernel_init_gdt_and_tss(void);

/**
 * Initialize memory paging subsystem
 */
void kernel_init_paging(void);

/**
 * Initialize all kernel devices and subsystems
 */
void kernel_init_devices(void);

/**
 * Initialize graphics system and return mouse interface
 * @return Pointer to initialized mouse interface, or NULL on failure
 */
struct mouse *kernel_init_graphics(void);

/**
 * Display initial boot message
 */
void kernel_display_boot_message(void);

/**
 * Enable PIC timer IRQ0
 */
void kernel_unmask_timer_irq(void);

#endif // KERNEL_INIT_H

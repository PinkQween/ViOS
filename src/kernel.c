#include "kernel.h"
#include "kernel/init.h"
#include "kernel/mainloop.h"
#include "gdt/gdt.h"
#include "task/tss.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "config.h"
#include "audio/audio.h"
#include "debug/simple_serial.h"

struct paging_4gb_chunk *kernel_chunk = 0;

void kernel_page()
{
    kernel_registers();

    if (kernel_chunk)
        paging_switch(kernel_chunk);
}

// GDT and TSS
struct tss tss;
struct gdt gdt_real[VIOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[VIOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},
    {.base = (uint32_t)(uintptr_t)&tss, .limit = sizeof(tss) - 1, .type = 0x89}};

void kernel_main()
{
    simple_serial_init();
    simple_serial_puts("DEBUG: Kernel starting...\n");

    kernel_init_gdt_and_tss();
    simple_serial_puts("DEBUG: GDT and TSS initialized\n");

    kernel_init_devices();
    simple_serial_puts("DEBUG: Devices initialized\n");

    kernel_init_paging();
    simple_serial_puts("DEBUG: Paging initialized\n");

    struct mouse *mouse = kernel_init_graphics();
    simple_serial_puts("DEBUG: Graphics initialized\n");

    kernel_display_boot_message();
    simple_serial_puts("DEBUG: Boot message displayed\n");

    kernel_unmask_timer_irq();
    simple_serial_puts("DEBUG: Timer IRQ unmasked\n");

    simple_serial_puts("DEBUG: Starting main loop...\n");
    kernel_run_main_loop(mouse);
}

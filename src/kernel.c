#include "kernel.h"
#include "kernel/init.h"
#include "kernel/mainloop.h"
#include "gdt/gdt.h"
#include "task/tss.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "drivers/io/storage/disk.h"
#include "idt/idt.h"
#include "isr80h/isr80h.h"
#include "drivers/input/keyboard/keyboard.h"
#include "config.h"
#include "drivers/output/audio/audio.h"
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
    {.base = 0x00, .limit = 0x00, .type = 0x00},       // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a}, // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92}, // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xfa}, // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2}, // User data segment
    {.base = 0x00, .limit = 0x00, .type = 0x00}        // TSS Segment (set at runtime)
};

void kernel_main()
{
    simple_serial_init();
    simple_serial_puts("DEBUG: Kernel starting...\n");
    simple_serial_puts("DEBUG: Kernel entry point reached successfully\n");

    // 1. GDT setup
    kernel_init_gdt_and_tss();
    simple_serial_puts("DEBUG: GDT initialized\n");

    // 2. Heap
    kheap_init();
    simple_serial_puts("DEBUG: Kernel heap initialized\n");

    // 3. Filesystem
    fs_init();
    simple_serial_puts("DEBUG: Filesystem initialized\n");

    // 4. Disk
    disk_search_and_init();
    simple_serial_puts("DEBUG: Disk initialized\n");

    // 5. IDT
    idt_init();
    simple_serial_puts("DEBUG: IDT initialized\n");

    // 6. TSS
    kernel_init_tss();
    simple_serial_puts("DEBUG: TSS initialized\n");

    // 7. Paging
    kernel_init_paging();
    simple_serial_puts("DEBUG: Paging initialized\n");

    // 8. ISR80H
    isr80h_register_commands();
    simple_serial_puts("DEBUG: ISR80H commands registered\n");

    // 9. Keyboard
    keyboard_init();
    simple_serial_puts("DEBUG: Keyboard initialized\n");

    // 10. Graphics
    struct mouse *mouse = kernel_init_graphics();
    simple_serial_puts("DEBUG: Graphics initialized\n");

    audio_init();

    virtual_audio_control(VIRTUAL_AUDIO_BEEP);

    // 11. Boot message
    kernel_display_boot_message();
    simple_serial_puts("DEBUG: Boot message displayed\n");

    // 12. Timer IRQ
    kernel_unmask_timer_irq();
    simple_serial_puts("DEBUG: Timer IRQ unmasked\n");

    // 13. Main loop
    simple_serial_puts("DEBUG: About to start main loop...\n");
    kernel_run_main_loop(mouse);
    simple_serial_puts("DEBUG: Main loop returned (this shouldn't happen)\n");
}
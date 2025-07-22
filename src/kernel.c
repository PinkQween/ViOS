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
#include "drivers/input/mouse/mouse.h"
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
    simple_serial_puts("Enterting kernel");
    kernel_init_gdt_and_tss();
    kheap_init();
    fs_init();
    disk_search_and_init();
    idt_init();
    kernel_init_tss();
    kernel_init_paging();
    isr80h_register_commands();
    keyboard_init();
    mouse_init();
    kernel_init_graphics();
    audio_init();
    virtual_audio_control(VIRTUAL_AUDIO_BEEP);
    kernel_display_boot_message();
    kernel_unmask_timer_irq();
    enable_interrupts(); // Enable interrupts after ALL initialization is complete
    kernel_run_main_loop();
}
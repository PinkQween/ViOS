#include "init.h"
#include "kernel.h"
#include "gdt/gdt.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "fs/file.h"
#include "drivers/io/storage/disk.h"
#include "idt/idt.h"
#include "task/tss.h"
#include "task/process.h"
#include "task/task.h"
#include "drivers/input/keyboard/keyboard.h"
#include "drivers/input/mouse/mouse.h"
#include "memory/paging/paging.h"
#include "isr80h/isr80h.h"
#include "panic/panic.h"
#include "drivers/io/io.h"
#include "config.h"
#include "string/string.h"
#include "drivers/output/audio/audio.h"
#include "debug/simple_serial.h"
#include "utils/utils.h"
#include "drivers/output/vigfx/vigfx.h"

// External declarations
extern struct tss tss;
extern struct gdt gdt_real[VIOS_TOTAL_GDT_SEGMENTS];
extern struct gdt_structured gdt_structured[VIOS_TOTAL_GDT_SEGMENTS];
extern struct paging_4gb_chunk *kernel_chunk;
extern struct idt_desc idt_descriptors[];
extern void *interrupt_pointer_table[];
extern void *interrupt_callbacks[];
extern void *get_timer_interrupt_handler_ptr(void);
extern void print_hex32(uint32_t value);

void kernel_init_gdt_and_tss(void)
{
    simple_serial_puts("DEBUG: Starting GDT and TSS initialization\n");
    // Zero the TSS before setting up the GDT (PeachOS order)
    memset(&tss, 0, sizeof(tss));
    simple_serial_puts("DEBUG: TSS memset done (before GDT setup)\n");
    gdt_structured[5].base = (uint32_t)&tss;
    gdt_structured[5].limit = sizeof(tss);
    gdt_structured[5].type = 0x89; // Available 32-bit TSS
    gdt_structured_to_gdt(gdt_real, gdt_structured, VIOS_TOTAL_GDT_SEGMENTS);
    simple_serial_puts("DEBUG: GDT structured to GDT conversion done\n");
    gdt_load(gdt_real, sizeof(gdt_real) - 1);
    simple_serial_puts("DEBUG: GDT loaded\n");
}

void kernel_init_tss(void)
{
    simple_serial_puts("DEBUG: Starting TSS setup\n");

    // memset(&tss, 0, sizeof(tss)); // Already done before GDT setup
    // simple_serial_puts("DEBUG: TSS memset done\n");

    // Allocate kernel stack for TSS (16KB stack)
    void *kernel_stack = kzalloc(16 * 1024);
    if (!kernel_stack)
    {
        simple_serial_puts("DEBUG: Failed to allocate kernel stack\n");
        panic("Failed to allocate kernel stack");
    }

    // Set TSS stack to allocated memory (add 16KB to point to top of stack)
    tss.esp0 = (uint32_t)kernel_stack + (16 * 1024);
    tss.ss0 = KERNEL_DATA_SELECTOR; // Ensure this matches any GDT data selectors
    simple_serial_puts("DEBUG: TSS fields set\n");

    simple_serial_puts("DEBUG: About to load TSS\n");
    char msg[64];
    snprintf(msg, sizeof(msg), "DEBUG: TSS address: %p\n", &tss);
    simple_serial_puts(msg);
    snprintf(msg, sizeof(msg), "DEBUG: Kernel stack allocated at: %p\n", kernel_stack);
    simple_serial_puts(msg);
    snprintf(msg, sizeof(msg), "DEBUG: TSS esp0 set to: %p\n", (void *)tss.esp0);
    simple_serial_puts(msg);
    snprintf(msg, sizeof(msg), "DEBUG: GDT address: %p\n", gdt_real);
    simple_serial_puts(msg);
    tss_load(0x28); // Ensure that 0x28 matches the TSS selector in the GDT
    simple_serial_puts("DEBUG: TSS loaded\n");
}

void kernel_init_paging(void)
{
    simple_serial_puts("DEBUG: Starting paging initialization\n");

    // Verify heap is working before paging
    void *test_alloc = kmalloc(4096);
    if (!test_alloc)
    {
        simple_serial_puts("DEBUG: Heap not ready for paging\n");
        panic("Heap not ready for paging");
    }
    kfree(test_alloc);
    simple_serial_puts("DEBUG: Heap verified working\n");

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!kernel_chunk)
    {
        simple_serial_puts("DEBUG: Failed to create kernel page directory\n");
        panic("Failed to create kernel page directory");
    }
    simple_serial_puts("DEBUG: Kernel page directory created\n");

    // Verify the page directory is valid
    if (!kernel_chunk->directory_entry)
    {
        simple_serial_puts("DEBUG: Invalid page directory\n");
        panic("Invalid page directory");
    }

    paging_switch(kernel_chunk);
    simple_serial_puts("DEBUG: Paging switched\n");

    enable_paging();
    simple_serial_puts("DEBUG: Paging enabled\n");
}

void kernel_init_graphics(void)
{
    graphics_init();
    
    global_mouse->x = gpu_screen_width() / 2;
    global_mouse->y = gpu_screen_height() / 2;   
}

void kernel_display_boot_message(void)
{
    simple_serial_puts("DEBUG: Displaying boot message\n");

    DRAW_CALLBACK bg_draw_func = NULL;
    void *bg_draw_ctx = NULL;

    getFuncToDrawScaledRGBFill("0:/sys/assets/logo.rgb", 0, 0, gpu_screen_width(), gpu_screen_height(), &bg_draw_func, &bg_draw_ctx);

    if (bg_draw_func)
    {
        bg_draw_func(bg_draw_ctx); // Draw background initially
    }
    else
    {
        simple_serial_puts("DEBUG: Failed to load background image\n");
        panic("Failed to load background image");
    }

    const int scale = 4.5;
    char* bootMessage = "Booting...";

    print(bootMessage, (gpu_screen_width() / 2) - (FONT_ATARIST8X16.width * scale * sizeof(bootMessage)), (gpu_screen_height() / 12 * 10) - (FONT_ATARIST8X16.height * scale), 255, 255, 255, scale, scale);

    gpu_flush_screen();

    simple_serial_puts("DEBUG: Boot message displayed\n");
}

void kernel_unmask_timer_irq(void)
{
    simple_serial_puts("DEBUG: Unmasking timer IRQ\n");

    // Print IDT entry for 0x20
    struct idt_desc *desc = &idt_descriptors[0x20];
    simple_serial_puts("DEBUG: IDT[0x20] offset_1: ");
    print_hex32(desc->offset_1);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: IDT[0x20] offset_2: ");
    print_hex32(desc->offset_2);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: IDT[0x20] selector: ");
    print_hex32(desc->selector);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: IDT[0x20] type_attr: ");
    print_hex32(desc->type_attr);
    simple_serial_puts("\n");

    // Print tss.esp0 and tss.ss0
    simple_serial_puts("DEBUG: tss.esp0: ");
    print_hex32(tss.esp0);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: tss.ss0: ");
    print_hex32(tss.ss0);
    simple_serial_puts("\n");

    // Print current stack pointer
    uint32_t esp_val = 0;
    asm volatile("mov %%esp, %0" : "=r"(esp_val));
    simple_serial_puts("DEBUG: Current ESP: ");
    print_hex32(esp_val);
    simple_serial_puts("\n");

    // Test writing to tss.esp0
    uint32_t *stack_ptr = (uint32_t *)tss.esp0;
    stack_ptr[-1] = 0xDEADBEEF;
    simple_serial_puts("DEBUG: Wrote 0xDEADBEEF to tss.esp0-4, read back: ");
    print_hex32(stack_ptr[-1]);
    simple_serial_puts("\n");

    uint8_t mask = inb(0x21);
    mask &= ~0x01; // Unmask IRQ0 (timer)
    outb(0x21, mask);
    simple_serial_puts("DEBUG: Timer IRQ unmasked\n");
}
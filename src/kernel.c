#include "kernel.h"
#include "terminal/terminal.h"
#include "gdt/gdt.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "idt/idt.h"
#include "task/tss.h"
#include "task/process.h"
#include "task/task.h"
#include "keyboard/keyboard.h"
#include "memory/paging/paging.h"
#include "isr80h/isr80h.h"
#include "string/string.h"
#include "panic/panic.h"
#include "status.h"

static struct paging_4gb_chunk *kernel_chunk = 0;
void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_chunk);
}

struct tss tss;
struct gdt gdt_real[VIOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[VIOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9}};

void kernel_main()
{
    terminal_initialize();
    print("Welcome to ViOS Kernel!\n\n\n\n");

    gdt_structured_to_gdt(gdt_real, gdt_structured, VIOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real) - 1);
    print_status("GDT loaded", "OK");

    kheap_init();
    print_status("Heap initialized", "OK");

    fs_init();
    print_status("Filesystem initialized", "OK");

    disk_search_and_init();
    print_status("Disks initialized", "OK");

    idt_init();
    print_status("Interrupt Descriptor Table initialized", "OK");

    memset(&tss, 0, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(0x28);
    print_status("TSS loaded", "OK");

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_chunk);
    enable_paging();
    print_status("Paging enabled", "OK");

    keyboard_init();
    print_status("Initialized all system keyboards", "OK");

    isr80h_register_commands();
    print_status("Registered kernel commands", "OK");

    struct process *process = 0;
    int res = process_load_switch("0:/shell.elf", &process);
    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load shell.elf\n");
    }

    task_run_first_ever_task();

    while (1)
    {
    }
}
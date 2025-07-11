#include "kernel.h"
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
#include "mouse/mouse.h"
#include "memory/paging/paging.h"
#include "isr80h/isr80h.h"
#include "string/string.h"
#include "panic/panic.h"
#include "status.h"
#include "graphics/graphics.h"
#include "fonts/characters_AtariST8x16SystemFont.h"
#include "fonts/characters_RobotoThin.h"
#include "math/fpu_math.h"
#include "mouse/ps2_mouse.h"
#include "fs/file.h"
#include "debug/serial.h"
#include "debug/klog.h"

int almost_equal(double a, double b, double epsilon)
{
    double diff = a - b;
    return (diff < 0 ? -diff : diff) < epsilon;
}

static struct paging_4gb_chunk *kernel_chunk = 0;
void kernel_page()
{
    kernel_registers();
    if (kernel_chunk)
    {
        paging_switch(kernel_chunk);
    }
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

static void kernel_setup_gdt_tss()
{
    gdt_structured_to_gdt(gdt_real, gdt_structured, VIOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real) - 1);

    memset(&tss, 0, sizeof(tss));
    tss.esp0 = 0x20000000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(0x28);
}

static void kernel_init_memory()
{
    kheap_init();
    fs_init();
    disk_search_and_init();
    idt_init();

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_chunk);
    enable_paging();
}

static void kernel_init_devices()
{
    keyboard_init();
    mouse_init();

    struct mouse *mouse = ps2_mouse_init();

    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    mouse->x = VBE->x_resolution / 2;
    mouse->y = VBE->y_resolution / 2;
}

static void kernel_init_graphics()
{
    if (!graphics_initialize())
    {
        panic("Failed to initialize graphics system");
    }
}

static void kernel_launch_first_process(const char *path)
{
    struct process *process = 0;
    int res = process_load_switch(path, &process);
    if (res != VIOS_ALL_OK)
    {
        panic("No first process: failed to load ");
    }
}

void kernel_main()
{
    // Initialize serial debugging
    serial_init(SERIAL_COM1_BASE, 115200);
    klog_init(KLOG_INFO, KLOG_DEST_SERIAL);
    klog_boot("Kernel booting...");
    
    KLOG_INFO("KERNEL", "Setting up GDT and TSS");
    kernel_setup_gdt_tss();
    
    KLOG_INFO("KERNEL", "Initializing memory subsystem");
    kernel_init_memory();
    
    KLOG_INFO("KERNEL", "Initializing devices");
    kernel_init_devices();
    
    KLOG_INFO("KERNEL", "Registering system call handlers");
    isr80h_register_commands();
    
    KLOG_INFO("KERNEL", "Initializing graphics subsystem");
    kernel_init_graphics();

    KLOG_INFO("KERNEL", "Loading first process: %s", "0:/systemd.elf");
    const char *first_program = "0:/systemd.elf";
    kernel_launch_first_process(first_program);

    KLOG_INFO("KERNEL", "Starting first task");
    task_run_first_ever_task();
    
    KLOG_INFO("KERNEL", "Kernel initialization complete");
}

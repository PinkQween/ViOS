#include "io/serial.h"
#include "kernel.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/gpt.h"
#include "disk/streamer.h"
#include "fs/file.h"
#include "fs/pparser.h"
#include "gdt/gdt.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "graphics/image/image.h"
#include "graphics/terminal.h"
#include "graphics/window.h"
#include "idt/idt.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"
#include "memory/heap/heap.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "status.h"
#include "string/string.h"
#include "task/process.h"
#include "task/task.h"
#include "task/tss.h"
#include "mouse/mouse.h"
#include "io/tsc.h"

// Global debug log for kernel and subsystems
void kernel_debug_log(const char* msg) {
    serial_write_string(SERIAL_COM1_BASE, msg);
}

struct terminal *system_terminal = NULL;

void terminal_writechar(char c, char colour)
{
    if (!system_terminal)
    {
        return;
    }

    terminal_write(system_terminal, c);
}

void print(const char *str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
}

void panic(const char *msg)
{
    print(msg);
    while (1)
    {
    }
}

struct tss tss;

extern struct gdt_entry gdt[];

// page descriptor
struct paging_desc *kernel_paging_desc = 0;

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_paging_desc);
}

struct paging_desc *kernel_desc() { return kernel_paging_desc; }

// defined in kernel.asm
extern struct graphics_info default_graphics_info;
void kernel_main()
{
    kernel_debug_log("[kernel_main] entered\n");

    struct graphics_info *screen_info = NULL;
    kernel_debug_log("[kernel_main] before kheap_init\n");
    kheap_init();
    kernel_debug_log("[kernel_main] after kheap_init\n");

    kernel_debug_log("[kernel_main] before paging_desc_new\n");
    kernel_paging_desc = paging_desc_new(PAGING_MAP_LEVEL_4);
    kernel_debug_log("[kernel_main] after paging_desc_new\n");

    if (!kernel_paging_desc)
    {
        panic("Failed to create kernel paging descriptor\n");
    }

    kernel_debug_log("[kernel_main] before paging_map_e820_memory_regions\n");
    paging_map_e820_memory_regions(kernel_paging_desc);
    kernel_debug_log("[kernel_main] after paging_map_e820_memory_regions\n");

    kernel_debug_log("[kernel_main] before paging_switch\n");
    paging_switch(kernel_paging_desc);
    kernel_debug_log("[kernel_main] after paging_switch\n");

    // The multi-heap is ready
    kheap_post_paging();

    // Setup the graphics
    graphics_setup(&default_graphics_info);

    screen_info = graphics_screen_info();

    // Enable interrupt descriptor table
    idt_init();

    // Enable fs functionality
    fs_init();

    // Enable the disks
    disk_search_and_init();

    // Initialize GPT(gloabl partition table) drives
    gpt_init();

    // Initialize the font system
    font_system_init();

    // Setup the terminal system
    terminal_system_setup();

    // Initialize the keyboard
    keyboard_init();

    // Heap allocation/free test before window_create
    {
        char buf[128];
        void* test_ptr = kzalloc(64);
        if (!test_ptr) {
            kernel_debug_log("[kernel_main] heap test alloc failed!\n");
        } else {
            unsigned long val = (unsigned long)test_ptr;
            int n = (sizeof(unsigned long) * 2) - 1;
            buf[0] = '['; buf[1] = 'h'; buf[2] = 't'; buf[3] = ':'; buf[4] = ' ';
            int i = 5;
            for (; n >= 0; n--) {
                int v = (val >> (n * 4)) & 0xF;
                buf[i++] = (v < 10) ? ('0' + v) : ('A' + v - 10);
            }
            buf[i++] = ']'; buf[i++] = '\n'; buf[i] = 0;
            kernel_debug_log(buf);
            kfree(test_ptr);
            kernel_debug_log("[kernel_main] heap test free done\n");
        }
    }
    // Initialize window system
    int win_init_res = window_system_initialize();
    if (win_init_res == 0) {
        kernel_debug_log("[kernel_main] window_system_initialize succeeded\n");
    } else {
        kernel_debug_log("[kernel_main] window_system_initialize failed\n");
    }

    // Initialize mouse system
    int mouse_init_res = mouse_system_init();
    if (mouse_init_res != 0) {
        panic("Failed to initialize mouse system\n");
    }
    kernel_debug_log("[kernel_main] after mouse_system_init\n");

    // Load the static mouse drivers.
    int mouse_drivers_res = mouse_system_load_static_drivers();
    if (mouse_drivers_res != 0) {
        panic("Failed to load static mouse drivers\n");
    }
    kernel_debug_log("[kernel_main] after mouse_system_load_static_drivers\n");

    // Enable interrupts so mouse can work
    enable_interrupts();
    kernel_debug_log("[kernel_main] interrupts enabled\n");

    // initialize stage two graphics setup - register mouse handlers
    graphics_setup_stage_two(&default_graphics_info);
    kernel_debug_log("[kernel_main] after graphics_setup_stage_two\n");

    // Initialize window system stage two
    window_system_initialize_stage2();
    kernel_debug_log("[kernel_main] after window_system_initialize_stage2\n");

    struct font *font = font_get_system_font();
    kernel_debug_log("[kernel_main] after font_get_system_font\n");
    if (!font)
    {
        panic("Failed to load system font\n");
    }

    struct framebuffer_pixel font_color = {0};
    font_color.red = 0xff;
    font_color.blue = 0xff;

    kernel_debug_log("[kernel_main] before terminal_create\n");
    system_terminal = terminal_create(screen_info, 0, 0, screen_info->width,
                                      screen_info->height, font, font_color,
                                      TERMINAL_FLAG_BACKSPACE_ALLOWED);
    kernel_debug_log("[kernel_main] after terminal_create\n");
    if (!system_terminal)
    {
        panic("Failed to create system terminal\n");
    }

    // Allocate a 1 MB stack for the kernel IDT
    size_t stack_size = 1024 * 1024;
    void *megabyte_stack_tss_end = kzalloc(stack_size);
    void *megabyte_stack_tss_begin = (void *)(((uintptr_t)megabyte_stack_tss_end) + stack_size);

    // Print and map main kernel stack (512KB)
    void* kernel_stack_base = (void*)0x00200000; // 2MB
    void* kernel_stack_top  = (void*)0x00280000; // 2.5MB
    {
        char buf[64];
        unsigned long val;
        int n;
        // Print base
        kernel_debug_log("[kernel stack] base: 0x");
        val = (unsigned long)kernel_stack_base;
        n = (sizeof(unsigned long) * 2) - 1;
        for (; n >= 0; n--) {
            int v = (val >> (n * 4)) & 0xF;
            buf[(sizeof(unsigned long) * 2) - 1 - n] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[sizeof(unsigned long) * 2] = 0;
        kernel_debug_log(buf);
        kernel_debug_log("\n");
        // Print top
        kernel_debug_log("[kernel stack] top:  0x");
        val = (unsigned long)kernel_stack_top;
        n = (sizeof(unsigned long) * 2) - 1;
        for (; n >= 0; n--) {
            int v = (val >> (n * 4)) & 0xF;
            buf[(sizeof(unsigned long) * 2) - 1 - n] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[sizeof(unsigned long) * 2] = 0;
        kernel_debug_log(buf);
        kernel_debug_log("\n");
    }
    // Map the kernel stack region as present and writable
    paging_map_to(kernel_desc(), kernel_stack_base, kernel_stack_base, kernel_stack_top, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT);

    // Block the first page
    paging_map(kernel_desc(), megabyte_stack_tss_end, megabyte_stack_tss_end, 0);

    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.rsp0 = (uint64_t)megabyte_stack_tss_begin;
    tss.iopb_offset = sizeof(tss); // No I/O permissions are used

    struct tss_desc_64 *tssdesc =
        (struct tss_desc_64 *)&gdt[KERNEL_LONG_MODE_TSS_GDT_INDEX];
    gdt_set_tss(tssdesc, &tss, sizeof(tss) - 1, TSS_DESCRIPTOR_TYPE, 0x00);

    // load the tss
    tss_load(KERNEL_LONG_MODE_TSS_SELECTOR);
    print("tss load was fine\n");

    // Register isr80h commands
    isr80h_register_commands();
    print("registered isr80h\n");

    // Initialize the keyboard
    // keyboard_init();

    kernel_debug_log("REACHED MAIN WINDOW CODE\n");
    kernel_debug_log("[kernel_main] creating main window\n");
    
    // Create a test window with title bar
    struct window *win = window_create(graphics_screen_info(), NULL, "Test Window", (size_t)100, (size_t)100, (size_t)200, (size_t)200, (uint64_t)0, (uint64_t)(-1));
    if (!win)
    {
        kernel_debug_log("[kernel_main] main window creation failed\n");
        print("Window creation problem\n");
    }
    else
    {
        kernel_debug_log("[kernel_main] main window created\n");
        terminal_print(window_terminal(win), "Hello, world!\n");
        kernel_debug_log("[kernel_main] terminal_print done\n");
        window_redraw(win);
        kernel_debug_log("[kernel_main] window_redraw done\n");
    }
   
   while(1) {}
    print("Loading program...\n");
    struct process* process = 0;
    int res = process_load_switch("@:/blank.elf", &process);
    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load user program\n");
    }

    // Drop to user land
    task_run_first_ever_task();

   

    while (1)
    {
    }
}
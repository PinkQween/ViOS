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
#include "debug/simple_serial.h"

int almost_equal(double a, double b, double epsilon)
{
    double diff = a - b;
    return (diff < 0 ? -diff : diff) < epsilon;
}

static struct paging_4gb_chunk *kernel_chunk = 0;
void kernel_page()
{
    kernel_registers();
    // Only switch if kernel_chunk is initialized
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

void kernel_main()
{
    simple_serial_init();
    simple_serial_puts("Serial Debug Initialized\n");
    gdt_structured_to_gdt(gdt_real, gdt_structured, VIOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real) - 1);
    simple_serial_puts("GDT loaded\n");
    kheap_init();
    simple_serial_puts("Heap initialized\n");
    fs_init();
    simple_serial_puts("Filesystem initialized\n");
    disk_search_and_init();
    simple_serial_puts("Disk initialized\n");
    idt_init();
    simple_serial_puts("IDT initialized\n");
    mouse_init();
    keyboard_init();
    simple_serial_puts("Input devices initialized\n");

    memset(&tss, 0, sizeof(tss));
    tss.esp0 = 0x20000000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(0x28);
    simple_serial_puts("TSS configured\n");
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_chunk);
    enable_paging();
    simple_serial_puts("Paging enabled\n");
    isr80h_register_commands();
    simple_serial_puts("Interrupt handlers registered\n");

    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    simple_serial_puts("VBE info accessed\n");

    // Center the mouse cursor using the mouse struct
    extern struct mouse *ps2_mouse_init();
    struct mouse *mouse = ps2_mouse_init();
    mouse->x = VBE->x_resolution / 2;
    mouse->y = VBE->y_resolution / 2;
    simple_serial_puts("Mouse cursor centered\n");

    // Initialize the professional graphics system
    if (!graphics_initialize()) {
        simple_serial_puts("Graphics initialization failed\n");
        panic("Failed to initialize graphics system");
    }
    simple_serial_puts("Graphics system initialized\n");
    
    // struct process *process = 0;
    // int res = process_load_switch("0:/shell.elf", &process);
    // if (res == VIOS_ALL_OK)
    // {
    //     task_run_first_ever_task();
    // } else {
    //     panic("No first process");
    // }

    // Initial screen setup
    ClearScreen(11, 25, 69);
    DrawAtariString("ViOS Graphics System Initialized", 10, 10, 255, 255, 255, 1);
    Flush();

    // Track previous mouse position for clearing
    static int prev_mouse_x = -1;
    static int prev_mouse_y = -1;
    
    // Track previous positions for animated rectangles
    static int prev_red_rect_x = -1;
    static int prev_green_rect_size = -1;
    
    // Main professional graphics loop
    while (1)
    {
        // Begin frame with professional timing
        graphics_begin_frame();
        
        // Clear screen when needed using professional graphics context
        GraphicsContext *ctx = graphics_get_context();
        if (ctx && ctx->needs_full_refresh) {
            ClearScreen(11, 25, 69);
            prev_mouse_x = -1; // Reset mouse tracking after full clear
            prev_mouse_y = -1;
            ctx->needs_full_refresh = false;
        }
        
        // Only update frame info occasionally to reduce string operations
        static char frame_info[64] = "Frame: ...";
        static int frame_update_counter = 0;
        if (frame_update_counter % 30 == 0) {  // Update every 30 frames
            uint32_t frame_count = graphics_get_frame_count();
            // Simple integer to string conversion for frame counter
            frame_info[0] = 'F'; frame_info[1] = 'r'; frame_info[2] = 'a'; frame_info[3] = 'm'; frame_info[4] = 'e'; frame_info[5] = ':'; frame_info[6] = ' ';
            
            // Convert frame count to string (simple method)
            int digits = 0;
            uint32_t temp = frame_count;
            if (temp == 0) digits = 1;
            else {
                while (temp > 0) {
                    temp /= 10;
                    digits++;
                }
            }
            
            for (int i = digits - 1; i >= 0; i--) {
                frame_info[7 + i] = '0' + (frame_count % 10);
                frame_count /= 10;
            }
            frame_info[7 + digits] = '\0';
        }
        frame_update_counter++;
        
        DrawAtariString(frame_info, 10, 10, 255, 255, 255, 1);
        DrawAtariString("ViOS Enhanced Graphics System Running", 10, 30, 200, 200, 200, 1);
        
        // Draw some simple animated graphics to demonstrate the system
        static int animation_counter = 0;
        animation_counter++;
        
        // Only update animations every few frames for better performance
        if (animation_counter % 2 == 0) {
            // Calculate new positions first
            int rect_x = 50 + ((animation_counter / 2) % 200);
            int rect_y = 60;
            int size = 20 + ((animation_counter / 2) % 20);
            
            // Clear previous red rectangle if needed
            if (prev_red_rect_x != -1 && prev_red_rect_x != rect_x) {
                DrawRect(prev_red_rect_x, rect_y, 50, 30, 11, 25, 69);
            }
            
            // Draw a moving rectangle
            DrawRect(rect_x, rect_y, 50, 30, 255, 100, 100);
            prev_red_rect_x = rect_x;
            
            // Clear previous green rectangle if needed
            if (prev_green_rect_size != -1 && prev_green_rect_size != size) {
                DrawRect(400, 150, prev_green_rect_size, prev_green_rect_size, 11, 25, 69);
            }
            
            // Draw a simple pulsing rectangle instead of ellipse (faster)
            DrawRect(400, 150, size, size, 100, 255, 100);
            prev_green_rect_size = size;
        }
        
        // Store current mouse position to avoid changes during drawing
        int current_mouse_x = mouse->x;
        int current_mouse_y = mouse->y;
        
        // Clear previous mouse position if it has moved
        if (prev_mouse_x != -1 && prev_mouse_y != -1 && 
            (prev_mouse_x != current_mouse_x || prev_mouse_y != current_mouse_y)) {
            // Draw a larger rectangle with background color to ensure complete clearing
            DrawRect(prev_mouse_x, prev_mouse_y, 12, 12, 11, 25, 69);
        }
        
        // Draw mouse cursor at current position
        DrawMouse(current_mouse_x, current_mouse_y, 255, 255, 255);
        
        // Update previous position
        prev_mouse_x = current_mouse_x;
        prev_mouse_y = current_mouse_y;

        // End frame with professional timing and buffer management
        graphics_end_frame();
        
        // Present the frame (handles double buffering and VSync)
        graphics_present();
    }
}
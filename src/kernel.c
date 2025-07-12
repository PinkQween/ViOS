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
#include "math/fpu_math.h"
#include "audio/audio.h"
#include "mouse/ps2_mouse.h"
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
    {.base = (uint32_t)(uintptr_t)&tss, .limit = sizeof(tss) - 1, .type = 0xE9}};

static void initialize_gdt_and_tss()
{
    gdt_structured_to_gdt(gdt_real, gdt_structured, VIOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real) - 1);

    memset(&tss, 0, sizeof(tss));
    tss.esp0 = 0x20000000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(0x28);
}

static void initialize_paging()
{
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_chunk);
    enable_paging();
}

static void initialize_devices()
{
    kheap_init();
    fs_init();
    disk_search_and_init();
    idt_init();
    keyboard_init();
    mouse_init();
    audio_init();
    isr80h_register_commands();
}

static struct mouse *initialize_graphics()
{
    struct mouse *mouse = ps2_mouse_init();

    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    mouse->x = VBE->x_resolution / 2;
    mouse->y = VBE->y_resolution / 2;

    if (!graphics_initialize())
    {
        panic("Failed to initialize graphics system");
    }

    return mouse;
}

static void draw_boot_message()
{
    ClearScreen(11, 25, 69);
    DrawAtariString("ViOS Graphics System Initialized", 10, 10, 255, 255, 255, 1);
    Flush();
}

static void update_frame_info(char *frame_info)
{
    static int frame_update_counter = 0;
    if (frame_update_counter++ % 30 != 0)
        return;

    uint32_t frame_count = graphics_get_frame_count();
    strcpy(frame_info, "Frame: ");

    // Simple integer to string
    int i = 7;
    uint32_t temp = frame_count;
    if (temp == 0)
        frame_info[i++] = '0';
    else
    {
        char buf[10];
        int digits = 0;
        while (temp > 0)
        {
            buf[digits++] = '0' + (temp % 10);
            temp /= 10;
        }
        while (digits--)
        {
            frame_info[i++] = buf[digits];
        }
    }
    frame_info[i] = '\0';
}

static void draw_animated_rects(int frame, int *prev_red_x, int *prev_green_size)
{
    if (frame % 2 != 0)
        return;

    int rect_x = 50 + ((frame / 2) % 200);
    int size = 20 + ((frame / 2) % 20);
    int rect_y = 60;

    if (*prev_red_x != -1 && *prev_red_x != rect_x)
    {
        DrawRect(*prev_red_x, rect_y, 50, 30, 11, 25, 69);
    }

    DrawRect(rect_x, rect_y, 50, 30, 255, 100, 100);
    *prev_red_x = rect_x;

    if (*prev_green_size != -1 && *prev_green_size != size)
    {
        DrawRect(400, 150, *prev_green_size, *prev_green_size, 11, 25, 69);
    }

    DrawRect(400, 150, size, size, 100, 255, 100);
    *prev_green_size = size;
}

static void update_mouse(struct mouse *mouse, int *prev_x, int *prev_y)
{
    int x = mouse->x, y = mouse->y;
    if (*prev_x != -1 && *prev_y != -1 &&
        (x != *prev_x || y != *prev_y))
    {
        DrawRect(*prev_x, *prev_y, 12, 12, 11, 25, 69);
    }

    DrawMouse(x, y, 255, 255, 255);
    *prev_x = x;
    *prev_y = y;
}

static void kernel_loop(struct mouse *mouse)
{
    static int animation_counter = 0;
    static int prev_mouse_x = -1;
    static int prev_mouse_y = -1;
    static int prev_red_rect_x = -1;
    static int prev_green_rect_size = -1;
    static char frame_info[64] = "Frame: ...";

    while (1)
    {
        graphics_begin_frame();

        GraphicsContext *ctx = graphics_get_context();
        if (ctx && ctx->needs_full_refresh)
        {
            ClearScreen(11, 25, 69);
            prev_mouse_x = -1;
            prev_mouse_y = -1;
            ctx->needs_full_refresh = false;
        }

        update_frame_info(frame_info);
        DrawAtariString(frame_info, 10, 10, 255, 255, 255, 1);
        DrawAtariString("ViOS Enhanced Graphics System Running", 10, 30, 200, 200, 200, 1);

        draw_animated_rects(animation_counter, &prev_red_rect_x, &prev_green_rect_size);
        update_mouse(mouse, &prev_mouse_x, &prev_mouse_y);

        animation_counter++;
        graphics_end_frame();
        graphics_present();
    }
}

void kernel_main()
{
    simple_serial_init();
    simple_serial_puts("Serial Debug Initialized\n");

    simple_serial_puts("Initializing GDT and TSS...\n");
    initialize_gdt_and_tss();
    simple_serial_puts("GDT and TSS initialized\n");

    simple_serial_puts("Initializing devices...\n");
    initialize_devices();
    simple_serial_puts("Devices initialized\n");

    simple_serial_puts("Initializing paging...\n");
    initialize_paging();
    simple_serial_puts("Paging enabled\n");

    simple_serial_puts("Initializing graphics system...\n");
    struct mouse *mouse = initialize_graphics();
    simple_serial_puts("Graphics system initialized\n");

    draw_boot_message();
    simple_serial_puts("Boot message drawn\n");

    virtual_audio_control(VIRTUAL_AUDIO_BEEP);
    simple_serial_puts("Audio beep triggered\n");

    simple_serial_puts("Entering kernel loop...\n");
    kernel_loop(mouse);
}
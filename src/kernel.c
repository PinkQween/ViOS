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
#include "audio/sb16.h"
#include "io/io.h"
#include "rtc/rtc.h"

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

static void pc_speaker_beep(uint32_t frequency, uint32_t duration_ms)
{
    // Enable PC speaker
    uint8_t port61 = inb(0x61);
    outb(0x61, port61 | 0x03);
    
    // Configure PIT channel 2
    outb(0x43, 0xB6);
    
    // Calculate and set divisor
    uint16_t divisor = 1193180 / frequency;
    outb(0x42, divisor & 0xFF);
    outb(0x42, (divisor >> 8) & 0xFF);
    
    // Keep beeping for specified duration
    if (duration_ms > 0) {
        sleep_ms(duration_ms);
        // Disable PC speaker
        outb(0x61, port61 & 0xFC);
    }
    // If duration is 0, keep beeping indefinitely
}

static void kernel_init_audio()
{
    // Test PC speaker first with a short beep
    pc_speaker_beep(1000, 500);  // 1000 Hz for 500ms
    
    // Then start continuous beep
    pc_speaker_beep(800, 0);     // 800 Hz continuously
    
    // Also try Sound Blaster initialization
    if (sb16_init())
    {
        sb16_set_master_volume(255);
        sb16_set_pcm_volume(255);
        sb16_play_beep(1000);
    }
}

static const char *kernel_get_boot_mode_program()
{
    // Default to CLI
    const char *fallback = "0:/terminal.elf";

    int fd = fopen("0:/vios.cfg", "r");
    if (fd < 0)
        return fallback;

    char line[128];
    int bytes_read = fread(line, 1, sizeof(line) - 1, fd);
    if (bytes_read > 0)
    {
        line[bytes_read] = '\0';
        if (strstr(line, "mode=gui") != 0)
        {
            fclose(fd);
            return "0:/desktop.elf";
        }
    }

    fclose(fd);
    return fallback;
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
    kernel_setup_gdt_tss();
    kernel_init_memory();
    kernel_init_devices();
    isr80h_register_commands();
    kernel_init_graphics();
    kernel_init_audio();

    const char *first_program = kernel_get_boot_mode_program();
    kernel_launch_first_process(first_program);

    task_run_first_ever_task();
}

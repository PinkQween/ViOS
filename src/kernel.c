#include "kernel.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "fs/file.h"
#include "fs/pparser.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "idt/idt.h"
#include "io/io.h"
#include "gdt/gdt.h"
#include "task/tss.h"
#include "isr80h/isr80h.h"
#include "config.h"
#include "status.h"
#include "task/task.h"
#include "task/process.h"

#include <stddef.h>
#include <stdint.h>

// VGA text buffer
static uint16_t *video_mem = (uint16_t *)0xB8000;
static uint16_t terminal_row = 0;
static uint16_t terminal_col = 0;

static struct paging_4gb_chunk *kernel_chunk = 0;

// Constructs a character with color for VGA buffer
uint16_t terminal_make_char(char c, char colour)
{
    return (colour << 8) | c;
}

// Writes character at a specific VGA position
void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

// Writes character at current terminal position
void terminal_writechar(char c, char colour)
{
    if (c == '\n')
    {
        terminal_row++;
        terminal_col = 0;
        return;
    }

    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col++;

    if (terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row++;
    }
}

// Clears screen and initializes terminal
void terminal_initialize()
{
    terminal_row = 0;
    terminal_col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x, y, ' ', 0);
        }
    }
}

// Prints a string to the screen
void print(const char *str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15); // White text
    }
}

void print_colored(const char *str, char fg, char bg)
{
    char colour = (bg << 4) | (fg & 0x0F);
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        terminal_writechar(str[i], colour);
    }
}

// Converts integer to ASCII string
void int_to_ascii(int num, char *str)
{
    int i = 0, is_negative = 0;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    while (num)
    {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse string
    for (int j = 0, k = i - 1; j < k; j++, k--)
    {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

// Prints file stat info
void print_stat(struct file_stat *stat)
{
    print("File Stat:\n");

    char size_str[32];
    int_to_ascii(stat->filesize, size_str);
    print("  Size: ");
    print(size_str);
    print(" bytes\n");

    char flags_str[32];
    int_to_ascii(stat->flags, flags_str);
    print("  Flags: ");
    print(flags_str);
    print("\n");
}

void print_status(const char *message, const char *status)
{
    char fg = VGA_COLOR_WHITE;
    char bg = VGA_COLOR_BLACK;

    // Apply standard colors based on status string
    if (strcmp(status, "OK") == 0)
    {
        bg = VGA_COLOR_GREEN;
    }
    else if (strcmp(status, "WARN") == 0)
    {
        bg = VGA_COLOR_YELLOW;
        fg = VGA_COLOR_BLACK;
    }
    else if (strcmp(status, "ERR") == 0 || strcmp(status, "FAIL") == 0)
    {
        bg = VGA_COLOR_RED;
    }
    else if (strcmp(status, "PANIC") == 0)
    {
        bg = VGA_COLOR_RED;
        fg = VGA_COLOR_BLACK;
    }
    else
    {
        bg = VGA_COLOR_CYAN; // fallback for unknown statuses
    }

    print(message);

    int pad = VGA_WIDTH - terminal_col - strlen(status) - 3;
    for (int i = 0; i < pad; i++)
    {
        terminal_writechar('.', VGA_COLOR_DARK_GREY);
    }

    print(" [");
    print_colored(status, fg, bg);
    print("]\n");
}

// Kernel panic handler
void panic(const char *msg)
{
    print_status("SYSTEM PANIC", "PANIC");
    print(msg);
    while (1)
    {
    }
}

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_chunk);
}

// TSS and GDT setup
struct tss tss;
struct gdt gdt_real[VIOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[VIOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                 // NULL
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel Code
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},           // Kernel Data
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},           // User Code
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},           // User Data
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} // TSS
};

// Entry point of the kernel
void kernel_main()
{
    terminal_initialize();
    print("Welcome to ViOS Kernel!\n\n\n\n");

    // Load GDT
    memset(gdt_real, 0, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, VIOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real));
    print_status("GDT loaded", "OK");

    // Heap
    kheap_init();
    print_status("Heap initialized", "OK");

    // Filesystem
    fs_init();
    print_status("Filesystem initialized", "OK");

    // Disks
    disk_search_and_init();
    print_status("Disks initialized", "OK");

    // IDT
    idt_init();
    print_status("Interrupt Descriptor Table initialized", "OK");

    // Setup TSS
    memset(&tss, 0, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(0x28);
    print_status("TSS loaded", "OK");

    // Paging
    kernel_chunk = paging_new_4gb(
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_chunk);
    enable_paging();
    print_status("Paging enabled", "OK");

    isr80h_register_commands();
    print_status("Registered kernel commands", "OK");

    // // Enable interrupts
    // enable_interrupts();
    // print_status("Interrupts enabled", "OK");

    struct process *process = 0;
    int res = process_load("0:/blank.bin", &process);
    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load blank.bin");
    }

    task_run_first_ever_task();

    // Idle loop
    while (1)
    {
    }
}

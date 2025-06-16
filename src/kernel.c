#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "disk/streamer.h"

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char colour)
{
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_writechar(char c, char colour)
{
    if (c == '\n')
    {
        terminal_row += 1;
        terminal_col = 0;
        return;
    }

    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1;
    if (terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row += 1;
    }
}
void terminal_initialize()
{
    video_mem = (uint16_t *)(0xB8000);
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

void print(const char *str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
}

static struct paging_4gb_chunk *kernel_chunk = 0;

void kernel_main()
{
    terminal_initialize();
    print("Welcome to ViOS Kernel!\n\n");

    // Initialize heap
    kheap_init();
    print("[OK] Heap initialized\n");

    // Initialize filesystems
    fs_init();
    print("[OK] Filesystem initialized\n");

    // Initialize disks
    disk_search_and_init();
    print("[OK] Disks initialized\n");

    // Initialize interrupts
    idt_init();
    print("[OK] Interrupt descriptor table initialized\n");

    // Setup paging
    kernel_chunk = paging_new_4gb(
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    enable_paging();
    print("[OK] Paging enabled\n");

    // Enable system interrupts
    enable_interrupts();
    print("[OK] Interrupts enabled\n");

    // Try opening a file
    int fd = fopen("0:/file.txt", "r");
    if (fd)
    {
        print("[OK] Opened file: 0:/file.txt\n");
        char buf[14];
        fread(buf, 13, 1, fd);
        buf[13] = 0x00;
        print("Contents: ");
        print(buf);
        print("\n");
    }
    else
    {
        print("[WARN] Could not open 0:/file.txt\n");
    }

    // Kernel idle loop
    while (1)
    {
    }
}
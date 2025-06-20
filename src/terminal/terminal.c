#include "terminal.h"
#include "string/string.h"
#include "io/io.h"
#include <stdint.h>

static uint16_t *video_mem = (uint16_t *)0xB8000;
static uint16_t terminal_row = 0;
static uint16_t terminal_col = 0;

#define UART_BASE 0x3F8

static inline int uart_transmitter_empty()
{
    return (inb(UART_BASE + 5) & 0x20) != 0;
}

void uart_putc(char c)
{
    while (!uart_transmitter_empty())
        ; // wait until transmitter holding register empty
    outb(UART_BASE, c);
}

uint8_t convert_color(int fg, int bg)
{
    return (bg << 4) | (fg);
}

uint16_t terminal_make_char(char c, char colour)
{
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_backspace()
{
    if (terminal_row == 0 && terminal_col == 0)
        return;

    if (terminal_col == 0)
    {
        terminal_row -= 1;
        terminal_col = VGA_WIDTH - 1;
    }
    else
    {
        terminal_col -= 1;
    }

    terminal_putchar(terminal_col, terminal_row, ' ', convert_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK));
}

void terminal_scroll()
{
    for (int y = 1; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            video_mem[(y - 1) * VGA_WIDTH + x] = video_mem[y * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putchar(x, VGA_HEIGHT - 1, ' ', convert_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK));
    }
    if (terminal_row > 0)
        terminal_row--;
}

void terminal_writechar(char c, char colour)
{
    // print to VGA text buffer as usual
    if (c == '\n')
    {
        terminal_row++;
        terminal_col = 0;
        if (terminal_row >= VGA_HEIGHT)
        {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
        uart_putc('\r'); // carriage return for UART terminals
        uart_putc('\n'); // newline for UART terminals
        return;
    }

    if (c == 0x08)
    {
        terminal_backspace();
        uart_putc(0x08); // send backspace to UART
        return;
    }

    terminal_putchar(terminal_col, terminal_row, c, colour);
    uart_putc(c); // output char to UART

    terminal_col++;

    if (terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT)
        {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
    }
}

void terminal_initialize()
{
    terminal_row = 0;
    terminal_col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x, y, ' ', convert_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK));
        }
    }
}

void print(const char *str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        terminal_writechar(str[i], convert_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
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

void print_status(const char *message, const char *status)
{
    char fg = VGA_COLOR_WHITE;
    char bg = VGA_COLOR_BLACK;

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
        bg = VGA_COLOR_BLACK;
    }

    print(message);

    int pad = VGA_WIDTH - terminal_col - strlen(status) - 3;
    for (int i = 0; i < pad; i++)
    {
        terminal_writechar('.', convert_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK));
    }

    print(" [");
    print_colored(status, fg, bg);
    print("]\n");
}

void print_hex(uint8_t val)
{
    const char *hex_chars = "0123456789ABCDEF";
    char hex[3];
    hex[0] = hex_chars[(val >> 4) & 0xF];
    hex[1] = hex_chars[val & 0xF];
    hex[2] = 0;
    print(hex);
}

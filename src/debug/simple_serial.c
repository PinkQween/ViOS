#include "simple_serial.h"
#include "io/io.h"

void simple_serial_init(void)
{
    // Disable interrupts
    outb(SERIAL_COM1_BASE + 1, 0x00);

    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_COM1_BASE + 3, 0x80);

    // Set divisor to 3 (38400 baud)
    outb(SERIAL_COM1_BASE + 0, 0x03);
    outb(SERIAL_COM1_BASE + 1, 0x00);

    // 8 bits, no parity, one stop bit
    outb(SERIAL_COM1_BASE + 3, 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_COM1_BASE + 2, 0xC7);

    // Enable auxiliary output 2 (used as interrupt line for COM1)
    outb(SERIAL_COM1_BASE + 4, 0x0B);
}

static int is_transmit_empty(void)
{
    return inb(SERIAL_COM1_BASE + 5) & 0x20;
}

void simple_serial_putc(char c)
{
    while (is_transmit_empty() == 0)
        ;
    outb(SERIAL_COM1_BASE, c);
}

void simple_serial_puts(const char *str)
{
    if (!str)
        return;

    while (*str)
    {
        simple_serial_putc(*str);
        str++;
    }
}

static char hex_digit(uint8_t nibble)
{
    return nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10);
}

void simple_serial_put_hex(uint32_t value)
{
    simple_serial_puts("0x");

    for (int i = 28; i >= 0; i -= 4)
    {
        uint8_t nibble = (value >> i) & 0xF;
        simple_serial_putc(hex_digit(nibble));
    }
}

void print_hex32(uint32_t value)
{
    char hex[11] = "0x00000000";
    for (int i = 0; i < 8; i++)
    {
        int shift = (7 - i) * 4;
        int digit = (value >> shift) & 0xF;
        hex[2 + i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    }
    simple_serial_puts(hex);
}
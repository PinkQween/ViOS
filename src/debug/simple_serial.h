#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <stdint.h>

// Serial port constants
#define SERIAL_COM1_BASE 0x3F8

// Very simple serial functions
void simple_serial_init(void);
void simple_serial_putc(char c);
void simple_serial_puts(const char *str);
void simple_serial_put_hex(uint32_t value);
void print_hex32(uint32_t value);

#endif // SIMPLE_SERIAL_H

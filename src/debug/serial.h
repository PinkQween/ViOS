#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

// Serial port definitions
#define SERIAL_COM1_BASE 0x3F8
#define SERIAL_COM2_BASE 0x2F8

// Serial port registers (relative to base)
#define SERIAL_DATA_REG         0x0
#define SERIAL_INTERRUPT_REG    0x1
#define SERIAL_FIFO_REG         0x2
#define SERIAL_LINE_CTRL_REG    0x3
#define SERIAL_MODEM_CTRL_REG   0x4
#define SERIAL_LINE_STATUS_REG  0x5
#define SERIAL_MODEM_STATUS_REG 0x6
#define SERIAL_SCRATCH_REG      0x7

// Line Control Register bits
#define SERIAL_LCR_DLAB         0x80  // Divisor Latch Access Bit
#define SERIAL_LCR_8BITS        0x03  // 8 data bits
#define SERIAL_LCR_1STOP        0x00  // 1 stop bit
#define SERIAL_LCR_NO_PARITY    0x00  // No parity

// Line Status Register bits
#define SERIAL_LSR_DATA_READY   0x01  // Data ready
#define SERIAL_LSR_EMPTY_TX     0x20  // Transmitter empty
#define SERIAL_LSR_EMPTY_DATA   0x40  // Transmitter empty & line idle

// Baud rate divisors
#define SERIAL_BAUD_115200      1
#define SERIAL_BAUD_57600       2
#define SERIAL_BAUD_38400       3
#define SERIAL_BAUD_19200       6
#define SERIAL_BAUD_9600        12

// Serial port structure
typedef struct {
    uint16_t base_port;
    uint32_t baud_rate;
    uint8_t initialized;
} serial_port_t;

// Initialize serial port
int serial_init(uint16_t port, uint32_t baud_rate);

// Basic I/O
void serial_putc(char c);
void serial_puts(const char *str);
char serial_getc(void);
int serial_can_read(void);
int serial_can_write(void);

// Formatted output
void serial_printf(const char *format, ...);
void serial_vprintf(const char *format, va_list args);

// Debug output with levels
typedef enum {
    SERIAL_DEBUG_TRACE = 0,
    SERIAL_DEBUG_INFO  = 1,
    SERIAL_DEBUG_WARN  = 2,
    SERIAL_DEBUG_ERROR = 3,
    SERIAL_DEBUG_FATAL = 4
} serial_debug_level_t;

void serial_debug(serial_debug_level_t level, const char *subsystem, const char *format, ...);
void serial_set_debug_level(serial_debug_level_t min_level);

// Hex dump functionality
void serial_hex_dump(const void *data, size_t size, uint32_t base_address);

// Boot logging (available very early)
void serial_boot_log(const char *message);

// Convenience macros
#define SERIAL_TRACE(subsystem, ...) serial_debug(SERIAL_DEBUG_TRACE, subsystem, __VA_ARGS__)
#define SERIAL_INFO(subsystem, ...)  serial_debug(SERIAL_DEBUG_INFO,  subsystem, __VA_ARGS__)
#define SERIAL_WARN(subsystem, ...)  serial_debug(SERIAL_DEBUG_WARN,  subsystem, __VA_ARGS__)
#define SERIAL_ERROR(subsystem, ...) serial_debug(SERIAL_DEBUG_ERROR, subsystem, __VA_ARGS__)
#define SERIAL_FATAL(subsystem, ...) serial_debug(SERIAL_DEBUG_FATAL, subsystem, __VA_ARGS__)

#endif // SERIAL_H

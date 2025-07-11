#include "serial.h"
#include "io/io.h"
#include "string/string.h"
#include "rtc/rtc.h"
#include <stddef.h>

// Global serial port state
static serial_port_t g_serial_port = {0};
static serial_debug_level_t g_min_debug_level = SERIAL_DEBUG_TRACE;

// Simple printf implementation for kernel use
static void serial_print_number(uint32_t num, int base, int uppercase);
static void serial_print_hex_byte(uint8_t byte);

int serial_init(uint16_t port, uint32_t baud_rate)
{
    // Validate port
    if (port != SERIAL_COM1_BASE && port != SERIAL_COM2_BASE) {
        return -1;
    }
    
    g_serial_port.base_port = port;
    g_serial_port.baud_rate = baud_rate;
    
    // Disable interrupts
    outb(port + SERIAL_INTERRUPT_REG, 0x00);
    
    // Enable DLAB (set baud rate divisor)
    outb(port + SERIAL_LINE_CTRL_REG, SERIAL_LCR_DLAB);
    
    // Set divisor based on baud rate
    uint16_t divisor;
    switch (baud_rate) {
        case 115200: divisor = SERIAL_BAUD_115200; break;
        case 57600:  divisor = SERIAL_BAUD_57600;  break;
        case 38400:  divisor = SERIAL_BAUD_38400;  break;
        case 19200:  divisor = SERIAL_BAUD_19200;  break;
        case 9600:   divisor = SERIAL_BAUD_9600;   break;
        default:     divisor = SERIAL_BAUD_9600;   break;
    }
    
    outb(port + SERIAL_DATA_REG, divisor & 0xFF);           // Low byte
    outb(port + SERIAL_INTERRUPT_REG, (divisor >> 8) & 0xFF); // High byte
    
    // Configure line: 8 bits, no parity, 1 stop bit
    outb(port + SERIAL_LINE_CTRL_REG, SERIAL_LCR_8BITS | SERIAL_LCR_1STOP | SERIAL_LCR_NO_PARITY);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outb(port + SERIAL_FIFO_REG, 0xC7);
    
    // Enable auxilliary output 2 (used as interrupt line for COM1)
    outb(port + SERIAL_MODEM_CTRL_REG, 0x0B);
    
    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(port + SERIAL_MODEM_CTRL_REG, 0x1E);
    outb(port + SERIAL_DATA_REG, 0xAE);
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(port + SERIAL_DATA_REG) != 0xAE) {
        return -1;
    }
    
    // If serial is not faulty set it in normal operation mode
    outb(port + SERIAL_MODEM_CTRL_REG, 0x0F);
    
    g_serial_port.initialized = 1;
    
    // Send initialization message
    serial_puts("\r\n=== ViOS Serial Debug Log ===\r\n");
    serial_printf("Serial port initialized: COM%d at %d baud\r\n", 
                  (port == SERIAL_COM1_BASE) ? 1 : 2, baud_rate);
    
    return 0;
}

int serial_can_write(void)
{
    if (!g_serial_port.initialized) return 0;
    return inb(g_serial_port.base_port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_EMPTY_TX;
}

int serial_can_read(void)
{
    if (!g_serial_port.initialized) return 0;
    return inb(g_serial_port.base_port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_DATA_READY;
}

void serial_putc(char c)
{
    if (!g_serial_port.initialized) return;
    
    // Wait for transmitter to be empty
    while (!serial_can_write());
    
    // Convert LF to CRLF for proper terminal display
    if (c == '\n') {
        outb(g_serial_port.base_port + SERIAL_DATA_REG, '\r');
        while (!serial_can_write());
    }
    
    outb(g_serial_port.base_port + SERIAL_DATA_REG, c);
}

void serial_puts(const char *str)
{
    if (!str) return;
    
    while (*str) {
        serial_putc(*str++);
    }
}

char serial_getc(void)
{
    if (!g_serial_port.initialized) return 0;
    
    while (!serial_can_read());
    return inb(g_serial_port.base_port + SERIAL_DATA_REG);
}

// Simple printf implementation
void serial_vprintf(const char *format, va_list args)
{
    if (!format) return;
    
    const char *p = format;
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    if (num < 0) {
                        serial_putc('-');
                        num = -num;
                    }
                    serial_print_number(num, 10, 0);
                    break;
                }
                case 'u':
                    serial_print_number(va_arg(args, unsigned int), 10, 0);
                    break;
                case 'x':
                    serial_print_number(va_arg(args, unsigned int), 16, 0);
                    break;
                case 'X':
                    serial_print_number(va_arg(args, unsigned int), 16, 1);
                    break;
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    serial_puts("0x");
                    serial_print_number((uint32_t)ptr, 16, 0);
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);
                    serial_puts(str ? str : "(null)");
                    break;
                }
                case 'c':
                    serial_putc((char)va_arg(args, int));
                    break;
                case '%':
                    serial_putc('%');
                    break;
                default:
                    serial_putc('%');
                    serial_putc(*p);
                    break;
            }
        } else {
            serial_putc(*p);
        }
        p++;
    }
}

void serial_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    serial_vprintf(format, args);
    va_end(args);
}

void serial_set_debug_level(serial_debug_level_t min_level)
{
    g_min_debug_level = min_level;
}

void serial_debug(serial_debug_level_t level, const char *subsystem, const char *format, ...)
{
    if (level < g_min_debug_level) return;
    
    // Get current time
    struct rtc_time current_time;
    rtc_read(&current_time);
    
    // Print timestamp
    serial_printf("[%02d:%02d:%02d] ", 
                  current_time.hour, current_time.minute, current_time.second);
    
    // Print level
    const char *level_str;
    switch (level) {
        case SERIAL_DEBUG_TRACE: level_str = "TRACE"; break;
        case SERIAL_DEBUG_INFO:  level_str = "INFO "; break;
        case SERIAL_DEBUG_WARN:  level_str = "WARN "; break;
        case SERIAL_DEBUG_ERROR: level_str = "ERROR"; break;
        case SERIAL_DEBUG_FATAL: level_str = "FATAL"; break;
        default: level_str = "?????"; break;
    }
    
    serial_printf("[%s] ", level_str);
    
    // Print subsystem
    if (subsystem) {
        serial_printf("[%s] ", subsystem);
    }
    
    // Print message
    va_list args;
    va_start(args, format);
    serial_vprintf(format, args);
    va_end(args);
    
    serial_putc('\n');
}

void serial_hex_dump(const void *data, size_t size, uint32_t base_address)
{
    const uint8_t *bytes = (const uint8_t *)data;
    size_t i;
    
    serial_printf("Hex dump of %u bytes at 0x%08x:\n", size, base_address);
    
    for (i = 0; i < size; i += 16) {
        // Print address
        serial_printf("%08x: ", base_address + i);
        
        // Print hex bytes
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            serial_print_hex_byte(bytes[i + j]);
            serial_putc(' ');
            if (j == 7) serial_putc(' '); // Extra space in middle
        }
        
        // Pad if necessary
        for (size_t j = i + 16 - size; j > 0 && j <= 16; j--) {
            serial_puts("   ");
        }
        
        // Print ASCII
        serial_puts(" |");
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            uint8_t byte = bytes[i + j];
            serial_putc((byte >= 32 && byte <= 126) ? byte : '.');
        }
        serial_puts("|\n");
    }
}

void serial_boot_log(const char *message)
{
    // Early boot logging - works even before full initialization
    if (!g_serial_port.initialized) {
        // Try to initialize with basic settings
        serial_init(SERIAL_COM1_BASE, 9600);
    }
    
    serial_printf("[BOOT] %s\n", message);
}

// Helper functions
static void serial_print_number(uint32_t num, int base, int uppercase)
{
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char buffer[32];
    int i = 0;
    
    if (num == 0) {
        serial_putc('0');
        return;
    }
    
    while (num > 0) {
        buffer[i++] = digits[num % base];
        num /= base;
    }
    
    while (i > 0) {
        serial_putc(buffer[--i]);
    }
}

static void serial_print_hex_byte(uint8_t byte)
{
    const char *hex = "0123456789abcdef";
    serial_putc(hex[byte >> 4]);
    serial_putc(hex[byte & 0x0f]);
}

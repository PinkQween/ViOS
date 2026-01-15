#include "serial.h"
#include "io.h"

static inline void io_wait(void) {
    outb(0x80, 0);
}

void serial_init(uint16_t com) {
    outb(com + 1, 0x00);    // Disable all interrupts
    outb(com + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(com + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(com + 1, 0x00);    //                  (hi byte)
    outb(com + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(com + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(com + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    io_wait();
}

int serial_received(uint16_t com) {
    return insb(com + 5) & 1;
}

char serial_read(uint16_t com) {
    while (serial_received(com) == 0);
    return insb(com);
}

int serial_is_transmit_empty(uint16_t com) {
    return insb(com + 5) & 0x20;
}

void serial_write(uint16_t com, char a) {
    while (serial_is_transmit_empty(com) == 0);
    outb(com, a);
}

void serial_write_string(uint16_t com, const char* str) {
    while (*str) {
        serial_write(com, *str++);
    }
}

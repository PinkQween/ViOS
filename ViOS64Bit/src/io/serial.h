#ifndef VIOS_SERIAL_H
#define VIOS_SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define SERIAL_COM1_BASE                0x3F8      // COM1 base port
#define SERIAL_COM2_BASE                0x2F8      // COM2 base port

void serial_init(uint16_t com);
int serial_received(uint16_t com);
char serial_read(uint16_t com);
int serial_is_transmit_empty(uint16_t com);
void serial_write(uint16_t com, char a);
void serial_write_string(uint16_t com, const char* str);

#endif // VIOS_SERIAL_H

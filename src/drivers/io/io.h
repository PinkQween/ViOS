#ifndef IO_H
#define IO_H

#include <stdint.h>

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void insb(uint16_t port, void *buffer, uint32_t count);
void insw(uint16_t port, void *buffer, uint32_t count);

void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);

#endif
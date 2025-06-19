#ifndef IO_H
#define IO_H

unsigned char insb(unsigned short port);
unsigned short insw(unsigned short port);
unsigned char inb(unsigned short port);

void outb(unsigned short port, unsigned char val);
void outw(unsigned short port, unsigned short val);

#endif
#ifndef IO_H
#define IO_H

unsigned char inb(unsigned short port);
unsigned short insw(unsigned short port); // If you want to implement insw
void outb(unsigned short port, unsigned char val);
void outw(unsigned short port, unsigned short val);

#endif

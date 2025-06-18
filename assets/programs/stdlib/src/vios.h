#ifndef vios_H
#define vios_H

#include <stddef.h>
#include <stdbool.h>

void print(const char *filename);
int vios_getkey();
void vios_putchar(char c);

void *vios_malloc(size_t size);
void vios_free(void *ptr);
int vios_getkeyblock();
void vios_terminal_readline(char *out, int max, bool output_while_typing);

#endif
#ifndef VIOS_STDLIB_H
#define VIOS_STDLIB_H
#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
char *itoa(int i);
int atoi(const char *str);

#endif
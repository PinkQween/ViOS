#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

bool isDigit(char c);
int toNumericDigit(char c);
int strlen(const char *ptr);
int strnlen(const char *ptr, int max);
char *strcpy(char *dest, const char *src);

#endif
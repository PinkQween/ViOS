#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>

// int int_to_str(int value, char *buffer, int bufsize);
int strlen(const char *ptr);
int strnlen(const char *ptr, int max);
bool isdigit(char c);
int tonumericdigit(char c);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int count);
int strncmp(const char *str1, const char *str2, int n);
int istrncmp(const char *s1, const char *s2, int n);
int strnlen_terminator(const char *str, int max, char terminator);
char tolower(char s1);
int strcmp(const char *s1, const char *s2);
int snprintf(char *str, size_t size, const char *format, ...);
void print_dec(int n);

void int_to_str(int value, char *buffer);

void strcat(char *dest, const char *src);

char *strcpy_new(char *dest, const char *src);

char *strstr(const char *haystack, const char *needle);

#endif
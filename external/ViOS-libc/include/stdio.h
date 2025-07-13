#ifndef VIOS_STDIO_H
#define VIOS_STDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

    int printf(const char *fmt, ...);
    int sprintf(char *str, const char *fmt, ...);
    int snprintf(char *str, unsigned int size, const char *fmt, ...);
    int puts(const char *str);
    int putchar(int c);
    int getchar(void);

#ifdef __cplusplus
}
#endif

#endif
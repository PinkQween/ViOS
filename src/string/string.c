#include "string.h"
#include <stdarg.h> 

static int int_to_str(int value, char* buffer, int bufsize) {
    int pos = 0;
    bool negative = false;

    if (value < 0) {
        negative = true;
        value = -value;
    }

    // Write digits in reverse order
    char temp[20];
    int i = 0;
    do {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (negative) {
        temp[i++] = '-';
    }

    // Reverse into output buffer
    int len = i;
    if (bufsize > 0) {
        for (int j = 0; j < i && pos < bufsize; j++) {
            buffer[pos++] = temp[i - j - 1];
        }
    }

    return len; // length of string written (or would be)
}

char tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }

    return s1;
}

int strlen(const char *ptr)
{
    int i = 0;
    while (*ptr != 0)
    {
        i++;
        ptr += 1;
    }

    return i;
}

int strnlen(const char *ptr, int max)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (ptr[i] == 0)
            break;
    }

    return i;
}

int strnlen_terminator(const char *str, int max, char terminator)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (str[i] == '\0' || str[i] == terminator)
            break;
    }

    return i;
}

int istrncmp(const char *s1, const char *s2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}
int strncmp(const char *str1, const char *str2, int n)
{
    unsigned char u1, u2;

    while (n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

char *strcpy(char *dest, const char *src)
{
    char *res = dest;
    while (*src != 0)
    {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00;

    return res;
}

char *strncpy(char *dest, const char *src, int count)
{
    int i = 0;
    for (i = 0; i < count - 1; i++)
    {
        if (src[i] == 0x00)
            break;

        dest[i] = src[i];
    }

    dest[i] = 0x00;
    return dest;
}

bool isdigit(char c)
{
    return c >= 48 && c <= 57;
}
int tonumericdigit(char c)
{
    return c - 48;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    size_t pos = 0;  // position in output buffer
    int total_len = 0;  // total length that would have been written

    for (const char *p = format; *p != '\0'; p++) {
        if (*p != '%') {
            // Regular char
            if (pos + 1 < size) {
                str[pos] = *p;
            }
            pos++;
            total_len++;
        } else {
            p++;  // move past '%'

            if (*p == 's') {
                const char *s = va_arg(args, const char*);
                if (!s) s = "(null)";
                while (*s) {
                    if (pos + 1 < size) {
                        str[pos] = *s;
                    }
                    pos++;
                    total_len++;
                    s++;
                }
            } else if (*p == 'd') {
                int val = va_arg(args, int);
                char numbuf[32];
                int len = int_to_str(val, numbuf, sizeof(numbuf));
                for (int i = 0; i < len; i++) {
                    if (pos + 1 < size) {
                        str[pos] = numbuf[i];
                    }
                    pos++;
                    total_len++;
                }
            } else if (*p == '%') {
                if (pos + 1 < size) {
                    str[pos] = '%';
                }
                pos++;
                total_len++;
            } else {
                // Unsupported format, just print as-is
                if (pos + 1 < size) {
                    str[pos] = '%';
                }
                pos++;
                total_len++;
                if (*p != '\0') {
                    if (pos + 1 < size) {
                        str[pos] = *p;
                    }
                    pos++;
                    total_len++;
                }
            }
        }
    }

    if (size > 0) {
        if (pos < size) {
            str[pos] = '\0';
        } else {
            str[size - 1] = '\0';
        }
    }

    va_end(args);
    return total_len;
}
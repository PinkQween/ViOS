#include "string.h"
#include <stdarg.h>
#include <stdbool.h>

void int_to_str(int value, char *buffer)
{
    char temp[20];
    int i = 0, j = 0;
    int is_negative = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }

    while (value != 0)
    {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative)
    {
        temp[i++] = '-';
    }

    // reverse
    while (i > 0)
    {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}

char *strncpy_safe(char *dest, const char *src, size_t dest_size)
{
    if (dest_size == 0)
        return dest;
    char *res = dest;
    size_t i = 0;
    while (*src && i < dest_size - 1)
    {
        *dest++ = *src++;
        i++;
    }
    *dest = '\0';
    return res;
}

void strncat_safe(char *dest, const char *src, size_t dest_size)
{
    size_t dest_len = strnlen(dest, dest_size);
    if (dest_len >= dest_size - 1)
        return;

    char *end = dest + dest_len;
    size_t remaining = dest_size - dest_len - 1;

    while (*src)
    {
        if (remaining-- == 0)
            break;
        *end++ = *src++;
    }
    *end = '\0';
}

// int int_to_str(int value, char *buffer, int bufsize)
// {
//     int pos = 0;
//     bool negative = false;

//     if (value < 0)
//     {
//         negative = true;
//         value = -value;
//     }

//     char temp[20];
//     int i = 0;
//     do
//     {
//         temp[i++] = (value % 10) + '0';
//         value /= 10;
//     } while (value > 0);

//     if (negative)
//     {
//         temp[i++] = '-';
//     }

//     int len = i;
//     if (bufsize > 0)
//     {
//         for (int j = 0; j < i && pos < bufsize - 1; j++)
//         {
//             buffer[pos++] = temp[i - j - 1];
//         }
//         buffer[pos] = '\0';
//     }

//     return len;
// }

char tolower(char s1)
{
    if (s1 >= 'A' && s1 <= 'Z')
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

char *strcpy_new(char *dest, const char *src)
{
    char *res = dest;
    while (*src != 0)
    {
        *dest = *src;
        src += 1;
        dest += 1;
    }
    *dest = '\0';
    return res;
}

char *strncpy(char *dest, const char *src, int count)
{
    int i = 0;
    for (; i < count && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < count; i++)
        dest[i] = '\0';
    return dest;
}

bool isdigit(char c)
{
    return c >= '0' && c <= '9';
}

int tonumericdigit(char c)
{
    return c - '0';
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

    size_t pos = 0;
    int total_len = 0;

    for (const char *p = format; *p != '\0'; p++)
    {
        if (*p != '%')
        {
            if (pos + 1 < size)
            {
                str[pos] = *p;
            }
            pos++;
            total_len++;
        }
        else
        {
            p++;
            if (*p == 's')
            {
                const char *s = va_arg(args, const char *);
                if (!s)
                    s = "(null)";
                while (*s)
                {
                    if (pos + 1 < size)
                    {
                        str[pos] = *s;
                    }
                    pos++;
                    total_len++;
                    s++;
                }
            }
            else if (*p == 'd')
            {
                int val = va_arg(args, int);
                char numbuf[32];
                int_to_str(val, numbuf);
                int len = strlen(numbuf);
                for (int i = 0; i < len; i++)
                {
                    if (pos + 1 < size)
                    {
                        str[pos] = numbuf[i];
                    }
                    pos++;
                    total_len++;
                }
            }
            else if (*p == '%')
            {
                if (pos + 1 < size)
                {
                    str[pos] = '%';
                }
                pos++;
                total_len++;
            }
            else
            {
                if (pos + 1 < size)
                {
                    str[pos] = '%';
                }
                pos++;
                total_len++;
                if (*p != '\0')
                {
                    if (pos + 1 < size)
                    {
                        str[pos] = *p;
                    }
                    pos++;
                    total_len++;
                }
            }
        }
    }

    if (size > 0)
    {
        if (pos < size)
        {
            str[pos] = '\0';
        }
        else
        {
            str[size - 1] = '\0';
        }
    }

    va_end(args);
    return total_len;
}

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;
    while ((*dest++ = *src++))
    {
    }
    return ret;
}

void strcat(char *dest, const char *src)
{
    // Find the end of the destination string
    while (*dest != '\0')
    {
        dest++;
    }
    
    // Copy the source string to the end of destination
    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }
    
    // Null terminate
    *dest = '\0';
}
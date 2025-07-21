#include "utils.h"
#include <stdint.h>
#include "debug/simple_serial.h"

void int_to_ascii(int num, char *str)
{
    int i = 0, is_negative = 0;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    while (num)
    {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    for (int j = 0, k = i - 1; j < k; j++, k--)
    {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

int almost_equal(double a, double b, double epsilon)
{
    double diff = a - b;
    return (diff < 0 ? -diff : diff) < epsilon;
}

// Stub print_hex32 function
extern void print_hex32(uint32_t value);
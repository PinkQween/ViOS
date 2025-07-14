
#include "panic.h"
#include "debug/simple_serial.h"

void panic(const char *msg)
{
    simple_serial_puts(msg);
    while (1)
    {
    }
}
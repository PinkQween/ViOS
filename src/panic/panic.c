
#include "panic.h"
#include "terminal/terminal.h"

void panic(const char *msg)
{
    print_status("SYSTEM PANIC", "PANIC");
    print(msg);
    while (1)
    {
    }
}
#include "vios.h"

int vios_getkeyblock()
{
    int val = 0;
    do
    {
        val = vios_getkey();
    }
    while (val == 0);
    return val;
}

void vios_terminal_readline(char* out, int max, bool output_while_typing)
{
    int i = 0;
    for (i = 0; i < max - 1; i++)
    {
        char key = vios_getkeyblock();
        if (key == 13)
        {
            break;
        }

        if (output_while_typing)
        {
            vios_putchar(key);
        }

        if (key == 0x08 && i >= 1)
        {
            out[i-1] = 0x00;
            i -=2; // +1 when go to continue
            continue;
        }

        out[i] = key;
    }

    out[i] = 0x00;
}
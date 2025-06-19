#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "vios.h"

int main(int argc, char **argv)
{
    print("ViOS v1.0.0\n");
    while (1) {
        printf("> ");
        char buf[1024];
        vios_terminal_readline(buf, sizeof(buf), true);
        print("\n");
        vios_process_load_start(buf);
    }
    return 0;
}
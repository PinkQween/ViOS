#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "vios.h"
int main(int argc, char** argv)
{
    vios_print("VIOS v1.0.0\n");
    while(1) 
    {
        vios_print("> ");
        char buf[1024];
        vios_terminal_readline(buf, sizeof(buf), true);
        vios_print("\n");
        vios_system_run(buf);
        
        vios_print("\n");
    }
    return 0;
}

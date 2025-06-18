#include "vios.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char **argv)
{
    void *ptr = malloc(512);
    free(ptr);
    
    char buf[1024];
    vios_terminal_readline(buf, sizeof(buf), true);

    print(buf);

    while(1) {}
    return 0;
}
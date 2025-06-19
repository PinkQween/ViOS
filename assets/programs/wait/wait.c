#include "vios.h"
#include "stdio.h"

int main(int argc, char **argv)
{
    printf("0");
    vios_sleep(1);
    printf("1");
    vios_sleep(1);
    printf("2");
    vios_sleep(1);
    printf("3");
    return 0;
}
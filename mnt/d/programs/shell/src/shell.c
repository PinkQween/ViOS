#include "shell.h"
#include "vios.h"

int main(int argc, char **argv)
{
    vios_print("Test!", 0, 0, 255, 0, 0, 2);
    return 0;
}

#include "vios.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char path[64];
    snprintf(path, sizeof(path), "0:/%s", argv[1]);

    char *contents = vios_read(path);
    if (contents == NULL)
    {
        printf("Failed to read from path: %s\n", path);
        return 1;
    }

    printf("%s", contents);

    return 0;
}
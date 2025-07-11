#ifndef VIOS_STDLIB_H
#define VIOS_STDLIB_H
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void *malloc(size_t size);
    void free(void *ptr);
    char *itoa(int i);

#ifdef __cplusplus
}
#endif

#endif
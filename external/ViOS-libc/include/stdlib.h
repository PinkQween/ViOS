#ifndef VIOS_STDLIB_H
#define VIOS_STDLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

    void *malloc(unsigned int size);
    void free(void *ptr);
    void exit(int status);
    int atoi(const char *str);
    char *itoa(int i);
    int abs(int x);
    long labs(long x);
    void qsort(void *base, unsigned int nmemb, unsigned int size, int (*compar)(const void *, const void *));
    void *bsearch(const void *key, const void *base, unsigned int nmemb, unsigned int size, int (*compar)(const void *, const void *));

#ifdef __cplusplus
}
#endif

#endif

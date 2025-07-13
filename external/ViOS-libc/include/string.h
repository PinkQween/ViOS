#ifndef VIOS_STRING_H
#define VIOS_STRING_H

#ifdef __cplusplus
extern "C"
{
#endif

    unsigned int strlen(const char *str);
    int strcmp(const char *s1, const char *s2);
    int strncmp(const char *s1, const char *s2, unsigned int n);
    char *strcpy(char *dest, const char *src);
    char *strncpy(char *dest, const char *src, unsigned int n);
    char *strcat(char *dest, const char *src);
    char *strncat(char *dest, const char *src, unsigned int n);
    char *strchr(const char *s, int c);
    char *strrchr(const char *s, int c);
    char *strstr(const char *haystack, const char *needle);
    char *strtok(char *str, const char *delim);
    void *memset(void *s, int c, unsigned int n);
    void *memcpy(void *dest, const void *src, unsigned int n);
    int memcmp(const void *s1, const void *s2, unsigned int n);
    void *memchr(const void *s, int c, unsigned int n);
    char tolower(char c);
    char toupper(char c);
    int isdigit(int c);
    int isalpha(int c);
    int isalnum(int c);
    int isspace(int c);

#ifdef __cplusplus
}
#endif

#endif
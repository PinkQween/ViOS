#ifndef USERLAND_FILE_H
#define USERLAND_FILE_H

#include <stdint.h>
#include <stddef.h>

typedef unsigned int FILE_STAT_FLAGS;
struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

int fopen(const char* filename, const char* mode);
void fclose(int fd);
int fread(void* buffer, size_t size, size_t count, long fd);
int fwrite(const void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fseek(int fd, int offset, int whence);
int fstat(int fd, struct file_stat* file_stat_out);

#endif
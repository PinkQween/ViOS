#include "file.h"
#include "vios.h"

int fopen(const char* filename, const char* mode)
{
    return (int)vios_fopen(filename, mode);
}

void fclose(int fd)
{
    vios_fclose((size_t)fd);
}

int fread(void* buffer, size_t size, size_t count, long fd)
{
    return vios_fread(buffer, size, count, fd);
}

int fseek(int fd, int offset, int whence)
{
    return (int)vios_fseek(fd, offset, whence);
}

int fstat(int fd, struct file_stat* file_stat_out)
{
    return (int) vios_fstat(fd, file_stat_out);
}

int fwrite(const void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
    return (int)vios_fwrite(ptr, size, nmemb, fd);
}
#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "drivers/io/storage/disk.h"
#include "fat/fat16.h"
#include "fat/fat32.h"
#include "status.h"
#include "kernel.h"
#include "debug/simple_serial.h"

struct filesystem *filesystems[VIOS_MAX_FILESYSTEMS];
struct file_descriptor *file_descriptors[VIOS_MAX_FILE_DESCRIPTORS];

static struct filesystem **fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < VIOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}

void fs_insert_filesystem(struct filesystem *filesystem)
{
    struct filesystem **fs;
    fs = fs_get_free_filesystem();
    if (!fs)
    {
        while (1)
        {
        }
    }

    *fs = filesystem;
}

static void fs_static_load()
{
    fs_insert_filesystem(fat32_init());
    fs_insert_filesystem(fat16_init());
}

void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static void file_free_descriptor(struct file_descriptor *desc)
{
    file_descriptors[desc->index - 1] = 0x00;
    kfree(desc);
}

static int file_new_descriptor(struct file_descriptor **desc_out)
{
    int res = -ENOMEM;
    for (int i = 0; i < VIOS_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor *desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

static struct file_descriptor *file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= VIOS_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }

    // Descriptors start at 1
    int index = fd - 1;
    return file_descriptors[index];
}

struct filesystem *fs_resolve(struct disk *disk)
{
    simple_serial_puts("DEBUG: Entering fs_resolve\n");
    struct filesystem *fs = 0;
    char msg[64];
    simple_serial_puts("DEBUG: fs_resolve: before loop\n");
    for (int i = 0; i < VIOS_MAX_FILESYSTEMS; i++)
    {
        snprintf(msg, sizeof(msg), "DEBUG: fs_resolve: i=%d, fs=%p\n", i, filesystems[i]);
        simple_serial_puts(msg);
        if (filesystems[i])
        {
            snprintf(msg, sizeof(msg), "DEBUG: fs_resolve: filesystems[%d]->resolve=%p\n", i, filesystems[i]->resolve);
            simple_serial_puts(msg);
        }
        if (filesystems[i] && filesystems[i]->resolve)
        {
            simple_serial_puts("DEBUG: fs_resolve: about to call resolve\n");
            int res = filesystems[i]->resolve(disk);
            simple_serial_puts("DEBUG: fs_resolve: resolve returned\n");
            if (res == 0)
            {
                fs = filesystems[i];
                break;
            }
        }
    }
    simple_serial_puts("DEBUG: fs_resolve returning\n");
    return fs;
}

FILE_MODE file_get_mode_by_string(const char *str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0)
    {
        mode = FILE_MODE_READ;
    }
    else if (strncmp(str, "w", 1) == 0)
    {
        mode = FILE_MODE_WRITE;
    }
    else if (strncmp(str, "a", 1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }
    return mode;
}

int fopen(const char *filename, const char *mode_str)
{
    int res = 0;
    struct disk *disk = NULL;
    FILE_MODE mode = FILE_MODE_INVALID;
    void *descriptor_private_data = NULL;
    struct file_descriptor *desc = 0;
    struct path_root *root_path = pathparser_parse(filename, NULL);
    if (!root_path)
    {
        res = -EINVARG;
        goto out;
    }

    // We cannot have just a root path 0:/ 0:/test.txt
    if (!root_path->first)
    {
        res = -EINVARG;
        goto out;
    }

    // Ensure the disk we are reading from exists
    disk = disk_get(root_path->drive_no);
    if (!disk)
    {
        res = -EIO;
        goto out;
    }

    if (!disk->filesystem)
    {
        res = -EIO;
        goto out;
    }

    mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID)
    {
        res = -EINVARG;
        goto out;
    }

    descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data))
    {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    res = file_new_descriptor(&desc);
    if (res < 0)
    {
        goto out;
    }
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    if (res < 0)
    {
        // ERROR
        if (root_path)
        {
            pathparser_free(root_path);
            root_path = NULL;
        }

        if (disk && descriptor_private_data)
        {
            disk->filesystem->close(descriptor_private_data);
            descriptor_private_data = NULL;
        }

        if (desc)
        {
            file_free_descriptor(desc);
            desc = NULL;
        }

        // fopen shouldnt return negative values
        res = 0;
    }

    return res;
}

int fstat(int fd, struct file_stat *stat)
{
    int res = 0;
    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->stat(desc->disk, desc->private, stat);
out:
    return res;
}

int fclose(int fd)
{
    int res = 0;
    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->close(desc->private);
    if (res == VIOS_ALL_OK)
    {
        file_free_descriptor(desc);
    }
out:
    return res;
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    int res = 0;
    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->seek(desc->private, offset, whence);
out:
    return res;
}

int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd)
{
    int res = 0;
    if (size == 0 || nmemb == 0 || fd < 1)
    {
        res = -EINVARG;
        goto out;
    }

    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EINVARG;
        goto out;
    }

    res = desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char *)ptr);
out:
    return res;
}
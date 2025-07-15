#include "fat32.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "kernel.h"
#include <stdint.h>
#include "debug/simple_serial.h"

#define VIOS_FAT32_SIGNATURE 0x29
#define VIOS_FAT32_FAT_ENTRY_SIZE 0x04
#define VIOS_FAT32_BAD_SECTOR 0x0FFFFFF7
#define VIOS_FAT32_UNUSED 0x00

struct fat32_header_extended
{
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t version;
    uint32_t root_directory_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat32_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;
    struct fat32_header_extended extended;
} __attribute__((packed));

struct fat32_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t cluster_high;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t cluster_low;
    uint32_t filesize;
} __attribute__((packed));

struct fat32_directory
{
    struct fat32_directory_item *item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct fat32_item
{
    union
    {
        struct fat32_directory_item *item;
        struct fat32_directory *directory;
    };

    uint32_t type;
};

struct fat32_private
{
    struct fat32_header header;
    struct fat32_directory root_directory;

    struct disk_stream *cluster_read_stream;
    struct disk_stream *fat_read_stream;
    struct disk_stream *directory_stream;
};

int fat32_resolve(struct disk *disk);
void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode);
int fat32_read(struct disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr);
int fat32_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat32_stat(struct disk *disk, void *private, struct file_stat *stat);
int fat32_close(void *private);

struct filesystem fat32_fs =
    {
        .resolve = fat32_resolve,
        .open = fat32_open,
        .read = fat32_read,
        .seek = fat32_seek,
        .stat = fat32_stat,
        .close = fat32_close};

struct filesystem *fat32_init()
{
    strcpy(fat32_fs.name, "FAT32");
    return &fat32_fs;
}

static void fat32_init_private(struct disk *disk, struct fat32_private *private)
{
    memset(private, 0, sizeof(struct fat32_private));
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);
}

int fat32_resolve(struct disk *disk)
{
    simple_serial_puts("DEBUG: Entering fat32_resolve\n");
    int res = 0;
    struct fat32_private *fat_private = kzalloc(sizeof(struct fat32_private));
    fat32_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat32_fs;

    struct disk_stream *stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        res = -ENOMEM;
        goto out;
    }

    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != VIOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (fat_private->header.extended.signature != VIOS_FAT32_SIGNATURE)
    {
        res = -EFSNOTUS;
        goto out;
    }

    out:
    if (stream)
    {
        diskstreamer_close(stream);
    }

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    simple_serial_puts("DEBUG: fat32_resolve returning\n");
    return res;
}

void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode)
{
    // Placeholder implementation - returns error for now
    return ERROR(-EUNIMP);
}

int fat32_read(struct disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    // Placeholder implementation - returns error for now
    return -EUNIMP;
}

int fat32_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    // Placeholder implementation - returns error for now
    return -EUNIMP;
}

int fat32_stat(struct disk *disk, void *private, struct file_stat *stat)
{
    // Placeholder implementation - returns error for now
    return -EUNIMP;
}

int fat32_close(void *private)
{
    // Placeholder implementation - returns error for now
    return -EUNIMP;
}


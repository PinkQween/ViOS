#include "disk.h"
#include "io/io.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "debug/simple_serial.h"

struct disk disk;

int disk_read_sector(int lba, int total, void *buf)
{
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char)(lba & 0xff));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    unsigned short *ptr = (unsigned short *)buf;
    for (int b = 0; b < total; b++)
    {
        // Wait for the buffer to be ready
        char c = insb(0x1F7);
        while (!(c & 0x08))
        {
            c = insb(0x1F7);
        }

        // Copy from hard disk to memory
        for (int i = 0; i < 256; i++)
        {
            *ptr = insw(0x1F0);
            ptr++;
        }
    }
    return 0;
}

void disk_search_and_init()
{
    simple_serial_puts("DEBUG: Starting disk search and init\n");

    simple_serial_puts("DEBUG: About to memset disk\n");
    memset(&disk, 0, sizeof(disk));
    simple_serial_puts("DEBUG: Disk memset done\n");

    simple_serial_puts("DEBUG: Setting disk type\n");
    disk.type = VIOS_DISK_TYPE_REAL;
    simple_serial_puts("DEBUG: Disk type set\n");

    simple_serial_puts("DEBUG: Setting disk sector size\n");
    disk.sector_size = VIOS_SECTOR_SIZE;
    simple_serial_puts("DEBUG: Disk sector size set\n");

    simple_serial_puts("DEBUG: Setting disk id\n");
    disk.id = 0;
    simple_serial_puts("DEBUG: Disk id set\n");

    simple_serial_puts("DEBUG: About to call fs_resolve\n");
    disk.filesystem = fs_resolve(&disk);
    simple_serial_puts("DEBUG: fs_resolve completed\n");

    simple_serial_puts("DEBUG: Disk search and init completed\n");
}

struct disk *disk_get(int index)
{
    if (index != 0)
        return 0;

    return &disk;
}

int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf)
{
    if (idisk != &disk)
    {
        return -EIO;
    }

    return disk_read_sector(lba, total, buf);
}

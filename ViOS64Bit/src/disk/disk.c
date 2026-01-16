#include "disk.h"
#include "config.h"
#include "io/io.h"
#include "kernel.h"
#include "lib/vector/vector.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "string/string.h"

struct vector *disk_vector = NULL;

// A pointer to the primary hard disk
// allowing IO directly to the disk from LBA zero onwards
struct disk *disk = NULL;

// A pointer to the virtual disk that contains the primary kernel filesystem
// where kernel files are found.
struct disk *primary_fs_disk = NULL;


int disk_read_sector(int lba, int total, void *buf) {
  // Wait for the disk not to be busy
  while (insb(0x1F7) & 0x80) {
    // spin
  }

  // Select drive: bits 7..4=0xE, bits 3..0 = high nibble of lba
  outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
  // Sector count
  outb(0x1F2, (unsigned char)total);
  // LBA low, mid, high
  outb(0x1F3, (unsigned char)(lba & 0xff));
  outb(0x1F4, (unsigned char)((lba >> 8) & 0xff));
  outb(0x1F5, (unsigned char)((lba >> 16) & 0xff));

  // Read SECTORS command (0x20)
  outb(0x1F7, 0x20);

  unsigned short *ptr = (unsigned short *)buf;
  for (int b = 0; b < total; b++) {
    // Wait for the disk not to be busy
    while (insb(0x1F7) & 0x80) {
      // spin
    }

    // Check error bit
    char status = insb(0x1F7);
    if (status & 0x01) {
      return -EIO;
    }

    // Wait for the buffer to be ready
    while (!(insb(0x1F7) & 0x08))

    {
      // spin
    }

    // Copy from hard disk to memory
    for (int word = 0; word < 256; word++) {
      *ptr++ = insw(0x1F0);
    }
  }
  return 0;
}

int disk_create_new(int type, int starting_lba, int ending_lba,
                    size_t sector_size, struct disk **disk_out) {
  int res = 0;
  struct disk *disk = kzalloc(sizeof(struct disk));
  if (!disk) {
    res = -ENOMEM;
    goto out;
  }
  disk->type = type;
  disk->id = vector_count(disk_vector);
  disk->sector_size = sector_size;
  disk->starting_lba = starting_lba;
  disk->ending_lba = ending_lba;

  // Not all disks have filesystems its not an error not to have one
  disk->filesystem = fs_resolve(disk);
  if (disk->filesystem) {
    char fs_name[11] = {0};
    char primary_drive_fs_name[11] = {0};
    strncpy(primary_drive_fs_name, VIOS_KERNEL_FILESYSTEM_NAME,
            strlen(VIOS_KERNEL_FILESYSTEM_NAME));
    // Is the disk the primary disk, lets check
    disk->filesystem->volume_name(disk->fs_private, fs_name, sizeof(fs_name));
    kernel_debug_log("[disk_new] disk ");
    kernel_debug_log(itoa(disk->id));
    kernel_debug_log(" has volume name: '");
    kernel_debug_log(fs_name);
    kernel_debug_log("'\n");
    kernel_debug_log("[disk_new] looking for: '");
    kernel_debug_log(primary_drive_fs_name);
    kernel_debug_log("'\n");
    if (strncmp(fs_name, primary_drive_fs_name, sizeof(fs_name)) == 0) {
      // Set the primary filesystem disk
      primary_fs_disk = disk;
      print("Primary FS disk set: ");
      print(itoa(disk->id));
      print("\n");
    } else {
      kernel_debug_log("[disk_new] volume names don't match\n");
    }
  } else {
    kernel_debug_log("[disk_new] disk ");
    kernel_debug_log(itoa(disk->id));
    kernel_debug_log(" has no filesystem\n");
  }

  if (disk_out) {
    *disk_out = disk;
  }
  vector_push(disk_vector, &disk);
out:
  return res;
}

void disk_search_and_init() {
  int res = 0;
  kernel_debug_log("[disk_search_and_init] starting\n");
  disk_vector = vector_new(sizeof(struct disk *), 4, 0);
  if (!disk_vector) {
    kernel_debug_log("[disk_search_and_init] vector creation failed\n");
    res = -ENOMEM;
    goto out;
  }

  kernel_debug_log("[disk_search_and_init] creating disk 0\n");
  res =
      disk_create_new(VIOS_DISK_TYPE_REAL, 0, 0, VIOS_SECTOR_SIZE, &disk);
  if (res < 0) {
    kernel_debug_log("[disk_search_and_init] disk_create_new failed: ");
    kernel_debug_log(itoa(res));
    kernel_debug_log("\n");
    goto out;
  }
  kernel_debug_log("[disk_search_and_init] disk 0 created successfully\n");
out:
  return;
}

struct disk *disk_primary() { return disk; }

struct disk *disk_primary_fs_disk() { return primary_fs_disk; }

struct disk *disk_get(int index) {
  size_t total_disks = vector_count(disk_vector);
  if (index >= (int)total_disks) {
    // out of bounds no such disk is loaded
    return NULL;
  }

  struct disk *disk = NULL;
  vector_at(disk_vector, index, &disk, sizeof(disk));
  return disk;
}

int disk_read_block(struct disk *idisk, unsigned int lba, int total,
                    void *buf) {
  size_t absolute_lba = idisk->starting_lba + lba;
  size_t absolute_ending_lba = absolute_lba + total;
  if (absolute_ending_lba > idisk->ending_lba) {
    // Is this the primary disk
    if (idisk->starting_lba != 0 && idisk->ending_lba != 0) {
      // Out of bounds, you cannot read over to other virtual disks
      return -EIO;
    }
  }

  return disk_read_sector(absolute_lba, total, buf);
}

int disk_write_sector(int lba, int total, const void *buf) {
  // Wait for the disk not to be busy
  while (insb(0x1F7) & 0x80) {
    // spin
  }

  // Select drive: bits 7..4=0xE, bits 3..0 = high nibble of lba
  outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
  // Sector count
  outb(0x1F2, (unsigned char)total);
  // LBA low, mid, high
  outb(0x1F3, (unsigned char)(lba & 0xff));
  outb(0x1F4, (unsigned char)((lba >> 8) & 0xff));
  outb(0x1F5, (unsigned char)((lba >> 16) & 0xff));

  // Write SECTORS command (0x30)
  outb(0x1F7, 0x30);

  const unsigned short *ptr = (const unsigned short *)buf;
  for (int b = 0; b < total; b++) {
    // Wait for the disk not to be busy
    while (insb(0x1F7) & 0x80) {
      // spin
    }

    // Check error bit
    char status = insb(0x1F7);
    if (status & 0x01) {
      return -EIO;
    }

    // Wait for the buffer to be ready
    while (!(insb(0x1F7) & 0x08)) {
      // spin
    }

    // Copy from memory to hard disk
    for (int word = 0; word < 256; word++) {
      outw(0x1F0, *ptr++);
    }

    // Flush cache after each sector write
    outb(0x1F7, 0xE7);
    
    // Wait for flush to complete
    while (insb(0x1F7) & 0x80) {
      // spin
    }
  }
  
  return 0;
}

int disk_write_block(struct disk *idisk, unsigned int lba, int total,
                     const void *buf) {
  size_t absolute_lba = idisk->starting_lba + lba;
  size_t absolute_ending_lba = absolute_lba + total;
  if (absolute_ending_lba > idisk->ending_lba) {
    // Is this the primary disk
    if (idisk->starting_lba != 0 && idisk->ending_lba != 0) {
      // Out of bounds, you cannot write over to other virtual disks
      return -EIO;
    }
  }

  return disk_write_sector(absolute_lba, total, buf);
}

int disk_get_count() {
  if (!disk_vector) {
    return 0;
  }
  return (int)vector_count(disk_vector);
}

int disk_flush(struct disk *idisk) {
  // Issue cache flush command (0xE7)
  outb(0x1F7, 0xE7);
  
  // Wait for flush to complete
  while (insb(0x1F7) & 0x80) {
    // spin
  }
  
  // Check for errors
  char status = insb(0x1F7);
  if (status & 0x01) {
    return -EIO;
  }
  
  return 0;
}

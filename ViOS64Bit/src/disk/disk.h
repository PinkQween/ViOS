#ifndef DISK_H
#define DISK_H

#include "fs/file.h"

typedef unsigned int VIOS_DISK_TYPE;

// Represents a real physical hard disk
#define VIOS_DISK_TYPE_REAL 0

// Specifies this disk represents a partion/virtual-disk
#define VIOS_DISK_TYPE_PARTITION 1

#define VIOS_KERNEL_FILESYSTEM_NAME "VIOS       "

struct disk {
  VIOS_DISK_TYPE type;
  int sector_size;

  // The id of the disk
  int id;

  struct filesystem *filesystem;

  // Set both to zero for the primary disk
  // all bounds checking is ignored if set to zero.
  size_t starting_lba;
  size_t ending_lba;

  // The private data of our filesystem
  void *fs_private;
};

// Initialize and search for disks
void disk_search_and_init();

// Create a new disk structure
int disk_create_new(int type, int starting_lba, int ending_lba, size_t sector_size, struct disk** disk_out);

// Get disk by index
struct disk* disk_get(int index);

// Read blocks from disk
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);

// Write blocks to disk
int disk_write_block(struct disk* idisk, unsigned int lba, int total, const void* buf);

// Get primary filesystem disk
struct disk* disk_primary_fs_disk();

// Get primary disk
struct disk* disk_primary();

// Get total number of disks
int disk_get_count();

// Flush disk write cache
int disk_flush(struct disk* idisk);

#endif

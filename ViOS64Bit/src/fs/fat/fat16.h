#ifndef FAT16_H
#define FAT16_H

#include "fs/file.h"

/**
 * Initialize FAT16 filesystem
 */
struct filesystem* fat16_init();

/**
 * Write data to a FAT16 file
 */
int fat16_write(struct disk *disk, void *descriptor, const void *ptr, uint32_t size, uint32_t nmemb);

/**
 * Truncate a file to specified size
 */
int fat16_truncate(struct disk *disk, void *descriptor, uint32_t size);

#endif
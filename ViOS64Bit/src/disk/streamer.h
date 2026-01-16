#ifndef DISKSTREAMER_H
#define DISKSTREAMER_H

#include "disk.h"

/**
 * Disk Stream Structure
 * Provides sequential access to disk data with automatic sector management
 */
struct disk_stream
{
    int pos;                    // Current position in bytes
    struct disk* disk;          // Associated disk
};

/**
 * Create a new disk stream for the specified disk ID
 * @param disk_id The disk identifier
 * @return Pointer to new disk stream or NULL on error
 */
struct disk_stream* diskstreamer_new(int disk_id);

/**
 * Create a new disk stream from an existing disk structure
 * @param disk Pointer to disk structure
 * @return Pointer to new disk stream or NULL on error
 */
struct disk_stream* diskstreamer_new_from_disk(struct disk* disk);

/**
 * Seek to a position in the disk stream
 * @param stream The disk stream
 * @param pos Position in bytes
 * @return 0 on success, negative error code on failure
 */
int diskstreamer_seek(struct disk_stream* stream, int pos);

/**
 * Read data from the disk stream
 * Automatically handles sector boundaries
 * @param stream The disk stream
 * @param out Output buffer
 * @param total Number of bytes to read
 * @return 0 on success, negative error code on failure
 */
int diskstreamer_read(struct disk_stream* stream, void* out, int total);

/**
 * Write data to the disk stream
 * Automatically handles sector boundaries and read-modify-write
 * @param stream The disk stream
 * @param in Input buffer
 * @param total Number of bytes to write
 * @return 0 on success, negative error code on failure
 */
int diskstreamer_write(struct disk_stream* stream, const void* in, int total);

/**
 * Get current position in the disk stream
 * @param stream The disk stream
 * @return Current position in bytes
 */
int diskstreamer_tell(struct disk_stream* stream);

/**
 * Flush any pending writes to disk
 * @param stream The disk stream
 * @return 0 on success, negative error code on failure
 */
int diskstreamer_flush(struct disk_stream* stream);

/**
 * Close and free a disk stream
 * @param stream The disk stream to close
 */
void diskstreamer_close(struct disk_stream* stream);

#endif

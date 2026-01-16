/**
 * Disk Streamer Implementation
 * 
 * Provides streaming read/write access to disk data with automatic
 * sector boundary handling and buffering.
 * 
 * Features:
 * - Sequential and random access
 * - Automatic sector alignment
 * - Read-modify-write for partial sector writes
 * - Position tracking
 */

#include "streamer.h"
#include "config.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"

#include <stdbool.h>

struct disk_stream *diskstreamer_new(int disk_id) {
  struct disk *disk = disk_get(disk_id);
  if (!disk) {
    return 0;
  }

  struct disk_stream *streamer = kzalloc(sizeof(struct disk_stream));
  streamer->pos = 0;
  streamer->disk = disk;
  return streamer;
}

struct disk_stream* diskstreamer_new_from_disk(struct disk* disk)
{
    struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));
    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

int diskstreamer_seek(struct disk_stream *stream, int pos) {
  stream->pos = pos;
  return 0;
}

int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
    int sector = stream->pos / VIOS_SECTOR_SIZE;
    int offset = stream->pos % VIOS_SECTOR_SIZE;
    int total_to_read = total;
    bool overflow = (offset+total_to_read) >= VIOS_SECTOR_SIZE;
    char buf[VIOS_SECTOR_SIZE];
    if (overflow)
    {
        total_to_read -= (offset+total_to_read) - VIOS_SECTOR_SIZE;
    }

    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0)
    {
        goto out;
    }

    for(int i = 0; i < total_to_read; i++)
    {
        *(char*)out++ = buf[offset+i];
    }

    stream->pos += total_to_read;
    if (overflow)
    {
        res = diskstreamer_read(stream, out, total-total_to_read);
    }
    
out:
    return res;
}

int diskstreamer_write(struct disk_stream* stream, const void* in, int total)
{
    int sector = stream->pos / VIOS_SECTOR_SIZE;
    int offset = stream->pos % VIOS_SECTOR_SIZE;
    int total_to_write = total;
    bool overflow = (offset+total_to_write) >= VIOS_SECTOR_SIZE;
    char buf[VIOS_SECTOR_SIZE];
    
    // If we're writing to the middle or end of a sector, we need to read it first
    if (offset != 0 || (overflow && total_to_write < VIOS_SECTOR_SIZE))
    {
        int res = disk_read_block(stream->disk, sector, 1, buf);
        if (res < 0)
        {
            return res;
        }
    }
    
    if (overflow)
    {
        total_to_write -= (offset+total_to_write) - VIOS_SECTOR_SIZE;
    }

    // Copy data into buffer
    for(int i = 0; i < total_to_write; i++)
    {
        buf[offset+i] = *(const char*)in++;
    }

    // Write the modified sector back
    int res = disk_write_block(stream->disk, sector, 1, buf);
    if (res < 0)
    {
        return res;
    }

    stream->pos += total_to_write;
    if (overflow)
    {
        res = diskstreamer_write(stream, in, total-total_to_write);
    }
    
    return res;
}

int diskstreamer_tell(struct disk_stream* stream)
{
    return stream->pos;
}

int diskstreamer_flush(struct disk_stream* stream)
{
    // Force cache flush by issuing a cache flush command
    // This is a no-op for now as we flush after each write
    // Could be extended to implement write buffering in the future
    return 0;
}

void diskstreamer_close(struct disk_stream *stream) { kfree(stream); }

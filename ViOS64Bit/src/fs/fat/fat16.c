#include "fat16.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "kernel.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "string/string.h"
#include <stdint.h>

#define VIOS_FAT16_SIGNATURE 0x29
#define VIOS_FAT16_FAT_ENTRY_SIZE 0x02
#define VIOS_FAT16_BAD_SECTOR 0xFF7
#define VIOS_FAT16_UNUSED 0x00

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

struct fat_header_extended {
  uint8_t drive_number;
  uint8_t win_nt_bit;
  uint8_t signature;
  uint32_t volume_id;
  uint8_t volume_id_string[11];
  uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header {
  uint8_t short_jmp_ins[3];
  uint8_t oem_identifier[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_copies;
  uint16_t root_dir_entries;
  uint16_t number_of_sectors;
  uint8_t media_type;
  uint16_t sectors_per_fat;
  uint16_t sectors_per_track;
  uint16_t number_of_heads;
  uint32_t hidden_setors;
  uint32_t sectors_big;
} __attribute__((packed));

struct fat_h {
  struct fat_header primary_header;
  union fat_h_e {
    struct fat_header_extended extended_header;
  } shared;
};

struct fat_directory_item {
  uint8_t filename[8];
  uint8_t ext[3];
  uint8_t attribute;
  uint8_t reserved;
  uint8_t creation_time_tenths_of_a_sec;
  uint16_t creation_time;
  uint16_t creation_date;
  uint16_t last_access;
  uint16_t high_16_bits_first_cluster;
  uint16_t last_mod_time;
  uint16_t last_mod_date;
  uint16_t low_16_bits_first_cluster;
  uint32_t filesize;
} __attribute__((packed));

struct fat_directory {
  struct fat_directory_item *item;
  int total;
  int sector_pos;
  int ending_sector_pos;
};

struct fat_item {
  union {
    struct fat_directory_item *item;
    struct fat_directory *directory;
  };

  FAT_ITEM_TYPE type;
  
  // For files: location of directory entry on disk (for updating metadata)
  int directory_sector;   // Sector containing the directory entry
  int directory_offset;   // Byte offset within the sector
};

struct fat_file_descriptor {
  struct fat_item *item;
  uint32_t pos;
};

struct fat_private {
  struct fat_h header;
  struct fat_directory root_directory;

  // Used to stream data clusters
  struct disk_stream *cluster_read_stream;
  // Used to stream the file allocation table
  struct disk_stream *fat_read_stream;

  // Used in situations where we stream the directory
  struct disk_stream *directory_stream;
  
  // FAT16 name
  char name[11];
};

int fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode);
int fat16_read(struct disk *disk, void *descriptor, uint32_t size,
               uint32_t nmemb, char *out_ptr);
int fat16_write(struct disk *disk, void *descriptor, const void *ptr, uint32_t size, uint32_t nmemb);
int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat16_stat(struct disk *disk, void *private, struct file_stat *stat);
int fat16_close(void *private);
int fat16_volume_name(void* private, char* name_out, size_t max);
int fat16_truncate(struct disk *disk, void *descriptor, uint32_t size);

struct filesystem fat16_fs = {.resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .write = fat16_write,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close,
    .volume_name=fat16_volume_name
};

struct filesystem *fat16_init() {
  strcpy(fat16_fs.name, "FAT16");
  return &fat16_fs;
}

static void fat16_init_private(struct disk *disk, struct fat_private *private) {
  memset(private, 0, sizeof(struct fat_private));
  private->cluster_read_stream = diskstreamer_new_from_disk(disk);
  private->fat_read_stream = diskstreamer_new_from_disk(disk);
  private->directory_stream = diskstreamer_new_from_disk(disk);
}

int fat16_sector_to_absolute(struct disk *disk, int sector) {
  return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(struct disk *disk,
                                        uint32_t directory_start_sector) {
  struct fat_directory_item item;
  struct fat_directory_item empty_item;
  memset(&empty_item, 0, sizeof(empty_item));

  struct fat_private *fat_private = disk->fs_private;

  int res = 0;
  int i = 0;
  int directory_start_pos = directory_start_sector * disk->sector_size;
  struct disk_stream *stream = fat_private->directory_stream;
  if (diskstreamer_seek(stream, directory_start_pos) != VIOS_ALL_OK) {
    res = -EIO;
    goto out;
  }

  while (1) {
    if (diskstreamer_read(stream, &item, sizeof(item)) != VIOS_ALL_OK) {
      res = -EIO;
      goto out;
    }

    if (item.filename[0] == 0x00) {
      // We are done
      break;
    }

    // Is the item unused
    if (item.filename[0] == 0xE5) {
      continue;
    }

    i++;
  }

  res = i;

out:
  return res;
}

int fat16_get_root_directory(struct disk *disk, struct fat_private *fat_private,
                             struct fat_directory *directory) {
  int res = 0;
  struct fat_directory_item *dir = 0x00;
  struct fat_header *primary_header = &fat_private->header.primary_header;
  int root_dir_sector_pos =
      (primary_header->fat_copies * primary_header->sectors_per_fat) +
      primary_header->reserved_sectors;
  int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
  int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
  int total_sectors = root_dir_size / disk->sector_size;
  if (root_dir_size % disk->sector_size) {
    total_sectors += 1;
  }

  dir = kzalloc(root_dir_size);
  if (!dir) {
    res = -ENOMEM;
    goto err_out;
  }

  struct disk_stream *stream = fat_private->directory_stream;
  if (diskstreamer_seek(stream,
                        fat16_sector_to_absolute(disk, root_dir_sector_pos)) !=
      VIOS_ALL_OK) {
    res = -EIO;
    goto err_out;
  }

  if (diskstreamer_read(stream, dir, root_dir_size) != VIOS_ALL_OK) {
    res = -EIO;
    goto err_out;
  }

  directory->item = dir;
  directory->total = root_dir_entries;  // Use max capacity, not current count
  directory->sector_pos = root_dir_sector_pos;
  directory->ending_sector_pos = root_dir_sector_pos + total_sectors;
out:
  return res;

err_out:
  if (dir) {
    kfree(dir);
  }

  return res;
}
int fat16_resolve(struct disk *disk) {
  int res = 0;
  struct fat_private *fat_private = kzalloc(sizeof(struct fat_private));
  fat16_init_private(disk, fat_private);

  disk->fs_private = fat_private;
  disk->filesystem = &fat16_fs;

  struct disk_stream *stream = diskstreamer_new_from_disk(disk);
  if (!stream) {
    res = -ENOMEM;
    goto out;
  }

  if (diskstreamer_read(stream, &fat_private->header,
                        sizeof(fat_private->header)) != VIOS_ALL_OK) {
    res = -EIO;
    goto out;
  }

  if (fat_private->header.shared.extended_header.signature != 0x29) {
    res = -EFSNOTUS;
    goto out;
  }

  if (fat16_get_root_directory(disk, fat_private,
                               &fat_private->root_directory) != VIOS_ALL_OK) {
    res = -EIO;
    goto out;
  }

  // Copy the name into the private data
  strncpy(fat_private->name, (const char*) fat_private->header.shared.extended_header.volume_id_string, sizeof(fat_private->name));

out:
  if (stream) {
    diskstreamer_close(stream);
  }

  if (res < 0) {
    kfree(fat_private);
    disk->fs_private = 0;
  }
  return res;
}

void fat16_to_proper_string(char **out, const char *in, size_t size) {
  int i = 0;
  while (*in != 0x00 && *in != 0x20) {
    **out = *in;
    *out += 1;
    in += 1;
    i++;
    // We cant process anymore since we have exceeded the input buffer size
    if (i >= size) {
      break;
    }
  }

  **out = 0x00;
}

void fat16_get_full_relative_filename(struct fat_directory_item *item,
                                      char *out, int max_len) {
  memset(out, 0x00, max_len);
  char *out_tmp = out;
  fat16_to_proper_string(&out_tmp, (const char *)item->filename,
                         sizeof(item->filename));
  if (item->ext[0] != 0x00 && item->ext[0] != 0x20) {
    *out_tmp++ = '.';
    fat16_to_proper_string(&out_tmp, (const char *)item->ext,
                           sizeof(item->ext));
  }
}

struct fat_directory_item *
fat16_clone_directory_item(struct fat_directory_item *item, int size) {
  struct fat_directory_item *item_copy = 0;
  if (size < sizeof(struct fat_directory_item)) {
    return 0;
  }

  item_copy = kzalloc(size);
  if (!item_copy) {
    return 0;
  }

  memcpy(item_copy, item, size);
  return item_copy;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item *item) {
  return (item->high_16_bits_first_cluster << 16) |
         item->low_16_bits_first_cluster;
};

static int fat16_cluster_to_sector(struct fat_private *private, int cluster) {
  return private->root_directory.ending_sector_pos +
         ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct fat_private *private) {
  return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct disk *disk, int cluster) {
  int res = -1;
  struct fat_private *private = disk->fs_private;
  struct disk_stream *stream = private->fat_read_stream;
  if (!stream) {
    goto out;
  }

  uint32_t fat_table_position =
      fat16_get_first_fat_sector(private) * disk->sector_size;
  res = diskstreamer_seek(stream, fat_table_position +
                                      (cluster * VIOS_FAT16_FAT_ENTRY_SIZE));
  if (res < 0) {
    goto out;
  }

  uint16_t result = 0;
  res = diskstreamer_read(stream, &result, sizeof(result));
  if (res < 0) {
    goto out;
  }

  res = result;
out:
  return res;
}

/**
 * Gets the correct cluster to use based on the starting cluster and the offset
 */
int fat16_get_cluster_for_offset(struct disk *disk, uint16_t starting_cluster,
                                 int offset) {
  int res = 0;
  struct fat_private *private = disk->fs_private;
  int size_of_cluster_bytes =
      private->header.primary_header.sectors_per_cluster * disk->sector_size;
  uint16_t cluster_to_use = starting_cluster;
  int clusters_ahead = offset / size_of_cluster_bytes;
  for (int i = 0; i < clusters_ahead; i++) {
    uint16_t entry = fat16_get_fat_entry(disk, cluster_to_use);
    if (entry >= 0xFFF8) {
      // End of cluster chain
      res = -EOUTOFRANGE;
      goto out;
    }

    // Check for other invalid or reserved entries
    if (entry == VIOS_FAT16_BAD_SECTOR ||
        (entry >= 0xFFF0 && entry <= 0xFFF6) || (entry == 0x0000)) {
      res = -EIO;
      goto out;
    }

    cluster_to_use = entry;
  }

  res = cluster_to_use;
out:
  return res;
}
static int fat16_read_internal_from_stream(struct disk *disk,
                                           struct disk_stream *stream,
                                           uint16_t cluster, int offset,
                                           int total, void *out) {
  int res = VIOS_ALL_OK;
  struct fat_private *private = disk->fs_private;
  int size_of_cluster_bytes =
      private->header.primary_header.sectors_per_cluster * disk->sector_size;
  uint16_t cluster_to_use = cluster;
  int bytes_read = 0;
  int starting_offset = offset;

  while (total > 0) {
    res = fat16_get_cluster_for_offset(disk, cluster, starting_offset);
    if (res < 0) {
      break;
    }

    cluster_to_use = (uint16_t)res;
    int offset_from_cluster = starting_offset % size_of_cluster_bytes;
    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos =
        (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = size_of_cluster_bytes - offset_from_cluster;
    if (total_to_read > total) {
      total_to_read = total;
    }

    res = diskstreamer_seek(stream, starting_pos);
    if (res != VIOS_ALL_OK) {
      break;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if (res != VIOS_ALL_OK) {
      break;
    }

    out += total_to_read;
    starting_offset += total_to_read;
    bytes_read += total_to_read;
    total -= total_to_read;
  }

  if (res < 0) {
    return res;
  }

  return bytes_read;
}

static int fat16_read_internal(struct disk *disk, int starting_cluster,
                               int offset, int total, void *out) {
  struct fat_private *fs_private = disk->fs_private;
  struct disk_stream *stream = fs_private->cluster_read_stream;
  return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset,
                                         total, out);
}

void fat16_free_directory(struct fat_directory *directory) {
  if (!directory) {
    return;
  }

  if (directory->item) {
    kfree(directory->item);
  }

  kfree(directory);
}

void fat16_fat_item_free(struct fat_item *item) {
  if (item->type == FAT_ITEM_TYPE_DIRECTORY) {
    fat16_free_directory(item->directory);
  } else if (item->type == FAT_ITEM_TYPE_FILE) {
    kfree(item->item);
  }

  kfree(item);
}

struct fat_directory *
fat16_load_fat_directory(struct disk *disk, struct fat_directory_item *item) {
  int res = 0;
  struct fat_directory *directory = 0;
  struct fat_private *fat_private = disk->fs_private;
  if (!(item->attribute & FAT_FILE_SUBDIRECTORY)) {
    res = -EINVARG;
    goto out;
  }

  directory = kzalloc(sizeof(struct fat_directory));
  if (!directory) {
    res = -ENOMEM;
    goto out;
  }

  int cluster = fat16_get_first_cluster(item);
  int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
  int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
  directory->total = total_items;
  int directory_size = directory->total * sizeof(struct fat_directory_item);
  directory->item = kzalloc(directory_size);
  if (!directory->item) {
    res = -ENOMEM;
    goto out;
  }

  res =
      fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
  if (res != VIOS_ALL_OK) {
    goto out;
  }

out:
  if (res != VIOS_ALL_OK) {
    fat16_free_directory(directory);
  }
  return directory;
}
struct fat_item *
fat16_new_fat_item_for_directory_item(struct disk *disk,
                                      struct fat_directory_item *item,
                                      int directory_sector,
                                      int directory_offset) {
  struct fat_item *f_item = kzalloc(sizeof(struct fat_item));
  if (!f_item) {
    return 0;
  }

  if (item->attribute & FAT_FILE_SUBDIRECTORY) {
    f_item->directory = fat16_load_fat_directory(disk, item);
    f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    f_item->directory_sector = directory_sector;
    f_item->directory_offset = directory_offset;
    return f_item;
  }

  f_item->type = FAT_ITEM_TYPE_FILE;
  f_item->item =
      fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
  f_item->directory_sector = directory_sector;
  f_item->directory_offset = directory_offset;
  return f_item;
}

struct fat_item *fat16_find_item_in_directory(struct disk *disk,
                                              struct fat_directory *directory,
                                              const char *name) {
  struct fat_item *f_item = 0;
  char tmp_filename[VIOS_MAX_PATH];
  kernel_debug_log("[fat16_find_item_in_directory] searching for: ");
  kernel_debug_log((char*)name);
  kernel_debug_log("\n");
  kernel_debug_log("[fat16_find_item_in_directory] checking ");
  kernel_debug_log(itoa(directory->total));
  kernel_debug_log(" entries\n");
  
  int checked = 0;
  for (int i = 0; i < directory->total; i++) {
    // Skip empty and deleted entries
    if (directory->item[i].filename[0] == 0x00) {
      break; // End of directory
    }
    if (directory->item[i].filename[0] == 0xE5) {
      continue; // Deleted entry
    }
    
    checked++;
    fat16_get_full_relative_filename(&directory->item[i], tmp_filename,
                                     sizeof(tmp_filename));
    
    // Debug: show what we're comparing
    if (checked > 16) {  // Only log newly created files
      kernel_debug_log("[fat16_find_item_in_directory] entry ");
      kernel_debug_log(itoa(i));
      kernel_debug_log(": ");
      kernel_debug_log(tmp_filename);
      kernel_debug_log("\n");
    }
    
    if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0) {
      kernel_debug_log("[fat16_find_item_in_directory] FOUND: ");
      kernel_debug_log(tmp_filename);
      kernel_debug_log("\n");
      // Found it - calculate directory entry location
      int entry_offset = i * sizeof(struct fat_directory_item);
      int sector_offset = entry_offset / disk->sector_size;
      int byte_offset = entry_offset % disk->sector_size;
      
      // Re-read the directory entry from disk to get the latest metadata
      struct fat_directory_item *fresh_item = kzalloc(sizeof(struct fat_directory_item));
      if (fresh_item) {
        struct fat_private *private = disk->fs_private;
        int position = (directory->sector_pos + sector_offset) * disk->sector_size + byte_offset;
        diskstreamer_seek(private->directory_stream, position);
        diskstreamer_read(private->directory_stream, fresh_item, sizeof(struct fat_directory_item));
        
        f_item = fat16_new_fat_item_for_directory_item(disk, fresh_item,
                                                        directory->sector_pos + sector_offset,
                                                        byte_offset);
        kfree(fresh_item);
      }
      break;
    }
  }

  kernel_debug_log("[fat16_find_item_in_directory] checked ");
  kernel_debug_log(itoa(checked));
  kernel_debug_log(" valid entries\n");
  
  if (!f_item) {
    kernel_debug_log("[fat16_find_item_in_directory] NOT FOUND\n");
  }
  
  return f_item;
}

struct fat_item *fat16_get_directory_entry(struct disk *disk,
                                           struct path_part *path) {
  struct fat_private *fat_private = disk->fs_private;
  struct fat_item *current_item = 0;
  struct fat_item *root_item = fat16_find_item_in_directory(
      disk, &fat_private->root_directory, path->part);
  if (!root_item) {
    goto out;
  }

  struct path_part *next_part = path->next;
  current_item = root_item;
  while (next_part != 0) {
    if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
      current_item = 0;
      break;
    }

    struct fat_item *tmp_item = fat16_find_item_in_directory(
        disk, current_item->directory, next_part->part);
    fat16_fat_item_free(current_item);
    current_item = tmp_item;
    next_part = next_part->next;
  }
out:
  return current_item;
}

/**
 * Create a new file in the root directory
 */
static struct fat_item *fat16_create_file_in_root(struct disk *disk, const char *filename) {
  kernel_debug_log("[fat16_create_file_in_root] start\n");
  struct fat_private *private = disk->fs_private;
  struct fat_directory *root_dir = &private->root_directory;
  int free_slot = -1;
  
  kernel_debug_log("[fat16_create_file_in_root] total entries: ");
  kernel_debug_log(itoa(root_dir->total));
  kernel_debug_log("\n");
  
  kernel_debug_log("[fat16_create_file_in_root] finding free slot\n");
  // Find a free slot in the root directory
  for (int i = 0; i < root_dir->total; i++) {
    if (root_dir->item[i].filename[0] == 0x00 || root_dir->item[i].filename[0] == 0xE5) {
      free_slot = i;
      kernel_debug_log("[fat16_create_file_in_root] found free slot: ");
      kernel_debug_log(itoa(i));
      kernel_debug_log("\n");
      break;
    }
  }
  
  if (free_slot < 0) {
    kernel_debug_log("[fat16_create_file_in_root] no free slot\n");
    return ERROR(-ENOMEM); // Root directory full
  }
  
  kernel_debug_log("[fat16_create_file_in_root] creating entry\n");
  // Create new directory entry
  struct fat_directory_item *new_item = &root_dir->item[free_slot];
  memset(new_item, 0, sizeof(struct fat_directory_item));
  
  kernel_debug_log("[fat16_create_file_in_root] parsing filename\n");
  // Parse filename (support only 8.3 format for now)
  char name[9] = {0};
  char ext[4] = {0};
  const char *dot = strchr(filename, '.');
  
  if (dot) {
    int name_len = dot - filename;
    if (name_len > 8) name_len = 8;
    strncpy(name, filename, name_len);
    strncpy(ext, dot + 1, 3);
  } else {
    strncpy(name, filename, 8);
  }
  
  kernel_debug_log("[fat16_create_file_in_root] converting to uppercase\n");
  // Convert to uppercase and pad with spaces
  memset(new_item->filename, ' ', 8);
  memset(new_item->ext, ' ', 3);
  for (int i = 0; i < 8 && name[i]; i++) {
    new_item->filename[i] = toupper(name[i]);
  }
  for (int i = 0; i < 3 && ext[i]; i++) {
    new_item->ext[i] = toupper(ext[i]);
  }
  
  kernel_debug_log("[fat16_create_file_in_root] setting attributes\n");
  new_item->attribute = FAT_FILE_ARCHIVED;
  new_item->filesize = 0;
  new_item->low_16_bits_first_cluster = 0;
  new_item->high_16_bits_first_cluster = 0;
  
  kernel_debug_log("[fat16_create_file_in_root] writing to disk\n");
  // Write directory entry to disk
  int root_sector = root_dir->sector_pos;
  int entry_offset = free_slot * sizeof(struct fat_directory_item);
  int sector_offset = entry_offset / disk->sector_size;
  int byte_offset = entry_offset % disk->sector_size;
  
  int position = (root_sector + sector_offset) * disk->sector_size + byte_offset;
  diskstreamer_seek(private->directory_stream, position);
  diskstreamer_write(private->directory_stream, new_item, sizeof(struct fat_directory_item));
  
  kernel_debug_log("[fat16_create_file_in_root] creating fat_item\n");
  // Create and return fat_item with directory location
  return fat16_new_fat_item_for_directory_item(disk, new_item, 
                                                root_sector + sector_offset,
                                                byte_offset);
}

void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode) {
  struct fat_file_descriptor *descriptor = 0;
  int err_code = 0;

  kernel_debug_log("[fat16_open] start\n");
  descriptor = kzalloc(sizeof(struct fat_file_descriptor));
  if (!descriptor) {
    kernel_debug_log("[fat16_open] kzalloc failed\n");
    err_code = -ENOMEM;
    goto err_out;
  }

  kernel_debug_log("[fat16_open] getting directory entry\n");
  descriptor->item = fat16_get_directory_entry(disk, path);
  kernel_debug_log("[fat16_open] got directory entry\n");
  
  // If file doesn't exist and we're in write mode, create it
  if (!descriptor->item && (mode == FILE_MODE_WRITE || mode == FILE_MODE_APPEND)) {
    kernel_debug_log("[fat16_open] file doesn't exist, creating\n");
    // Only support creating files in root directory for now
    if (!path->next) {
      kernel_debug_log("[fat16_open] calling fat16_create_file_in_root\n");
      descriptor->item = fat16_create_file_in_root(disk, path->part);
      kernel_debug_log("[fat16_open] fat16_create_file_in_root returned\n");
      if (ISERR(descriptor->item)) {
        kernel_debug_log("[fat16_open] create failed\n");
        err_code = ERROR_I(descriptor->item);
        descriptor->item = 0;
        goto err_out;
      }
    } else {
      kernel_debug_log("[fat16_open] subdirectory not supported\n");
      err_code = -EIO;
      goto err_out;
    }
  }
  
  if (!descriptor->item) {
    kernel_debug_log("[fat16_open] no item\n");
    err_code = -EIO;
    goto err_out;
  }

  descriptor->pos = 0;
  
  // For write mode, truncate the file to 0 bytes
  if (mode == FILE_MODE_WRITE) {
    if (descriptor->item->type == FAT_ITEM_TYPE_FILE) {
      fat16_truncate(disk, descriptor, 0);
    }
  }
  
  // For append mode, set position to end of file
  if (mode == FILE_MODE_APPEND) {
    if (descriptor->item->type == FAT_ITEM_TYPE_FILE) {
      descriptor->pos = descriptor->item->item->filesize;
    }
  }
  
  kernel_debug_log("[fat16_open] success\n");
  return descriptor;

err_out:
  kernel_debug_log("[fat16_open] error path\n");
  if (descriptor)
    kfree(descriptor);

  return ERROR(err_code);
}

static void fat16_free_file_descriptor(struct fat_file_descriptor *desc) {
  fat16_fat_item_free(desc->item);
  kfree(desc);
}

int fat16_close(void *private) {
  fat16_free_file_descriptor((struct fat_file_descriptor *)private);
  return 0;
}

int fat16_volume_name(void* private, char* name_out, size_t max)
{
    struct fat_private* fs_private = (struct fat_private*) private;
    strncpy(name_out, fs_private->name, max);
    return 0;
}

int fat16_stat(struct disk *disk, void *private, struct file_stat *stat) {
  int res = 0;
  struct fat_file_descriptor *descriptor =
      (struct fat_file_descriptor *)private;
  struct fat_item *desc_item = descriptor->item;
  if (desc_item->type != FAT_ITEM_TYPE_FILE) {
    res = -EINVARG;
    goto out;
  }

  struct fat_directory_item *ritem = desc_item->item;
  stat->filesize = ritem->filesize;
  stat->flags = 0x00;

  if (ritem->attribute & FAT_FILE_READ_ONLY) {
    stat->flags |= FILE_STAT_READ_ONLY;
  }
out:
  return res;
}

int fat16_read(struct disk *disk, void *descriptor, uint32_t size,
               uint32_t nmemb, char *out_ptr) {
  int res = 0;
  struct fat_file_descriptor *fat_desc = descriptor;
  struct fat_directory_item *item = fat_desc->item->item;
  int offset = fat_desc->pos;
  
  // Calculate how much we can actually read based on filesize
  uint32_t bytes_available = 0;
  if (offset < item->filesize) {
    bytes_available = item->filesize - offset;
  }
  
  uint32_t total_requested = size * nmemb;
  uint32_t total_to_read = total_requested < bytes_available ? total_requested : bytes_available;
  uint32_t items_to_read = total_to_read / size;
  
  if (items_to_read == 0) {
    return 0; // EOF or nothing to read
  }
  
  for (uint32_t i = 0; i < items_to_read; i++) {
    res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size,
                              out_ptr);
    if (ISERR(res)) {
      goto out;
    }

    out_ptr += size;
    offset += size;
  }
  fat_desc->pos = offset;
  res = items_to_read;
out:
  return res;
}

int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode) {
  int res = 0;
  struct fat_file_descriptor *desc = private;
  struct fat_item *desc_item = desc->item;
  if (desc_item->type != FAT_ITEM_TYPE_FILE) {
    res = -EINVARG;
    goto out;
  }

  struct fat_directory_item *ritem = desc_item->item;
  if (offset >= ritem->filesize) {
    res = -EIO;
    goto out;
  }

  switch (seek_mode) {
  case SEEK_SET:
    desc->pos = offset;
    break;

  case SEEK_END:
    res = -EUNIMP;
    break;

  case SEEK_CUR:
    desc->pos += offset;
    break;

  default:
    res = -EINVARG;
    break;
  }
out:
  return res;
}
// ============================================================================
// FAT16 WRITE OPERATIONS
// ============================================================================

/**
 * Write a FAT entry to the FAT table
 */
static int fat16_set_fat_entry(struct disk *disk, int cluster, uint16_t value) {
  struct fat_private *private = disk->fs_private;
  int fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
  fat_table_position += (cluster * VIOS_FAT16_FAT_ENTRY_SIZE);
  
  int res = 0;
  res = diskstreamer_seek(private->fat_read_stream, fat_table_position);
  if (res < 0) {
    goto out;
  }
  
  uint16_t entry = value;
  res = diskstreamer_write(private->fat_read_stream, &entry, sizeof(entry));
  if (res < 0) {
    goto out;
  }
  
  // Write to all FAT copies
  int fat_copies = private->header.primary_header.fat_copies;
  int sectors_per_fat = private->header.primary_header.sectors_per_fat;
  
  for (int i = 1; i < fat_copies; i++) {
    int copy_position = fat_table_position + (i * sectors_per_fat * disk->sector_size);
    res = diskstreamer_seek(private->fat_read_stream, copy_position);
    if (res < 0) {
      goto out;
    }
    res = diskstreamer_write(private->fat_read_stream, &entry, sizeof(entry));
    if (res < 0) {
      goto out;
    }
  }
  
out:
  return res;
}

/**
 * Find the next free cluster in the FAT table
 */
static int fat16_find_free_cluster(struct disk *disk) {
  struct fat_private *private = disk->fs_private;
  int total_sectors = private->header.primary_header.number_of_sectors;
  if (total_sectors == 0) {
    total_sectors = private->header.primary_header.sectors_big;
  }
  
  int sectors_per_cluster = private->header.primary_header.sectors_per_cluster;
  int total_clusters = total_sectors / sectors_per_cluster;
  
  // Start from cluster 2 (0 and 1 are reserved)
  for (int cluster = 2; cluster < total_clusters; cluster++) {
    int entry = fat16_get_fat_entry(disk, cluster);
    if (entry == VIOS_FAT16_UNUSED) {
      return cluster;
    }
  }
  
  return -ENOMEM;  // Disk full
}

/**
 * Allocate a new cluster and link it to the chain
 */
static int fat16_allocate_cluster(struct disk *disk, int previous_cluster) {
  int new_cluster = fat16_find_free_cluster(disk);
  if (new_cluster < 0) {
    return new_cluster;
  }
  
  // Mark new cluster as end of chain
  int res = fat16_set_fat_entry(disk, new_cluster, 0xFFFF);
  if (res < 0) {
    return res;
  }
  
  // Link previous cluster to new cluster if provided
  if (previous_cluster >= 0) {
    res = fat16_set_fat_entry(disk, previous_cluster, new_cluster);
    if (res < 0) {
      // Cleanup: free the allocated cluster
      fat16_set_fat_entry(disk, new_cluster, VIOS_FAT16_UNUSED);
      return res;
    }
  }
  
  return new_cluster;
}

/**
 * Free a cluster chain starting from the given cluster
 */
static int fat16_free_cluster_chain(struct disk *disk, int starting_cluster) {
  if (starting_cluster == 0) {
    return 0;  // Nothing to free
  }
  
  int current_cluster = starting_cluster;
  int res = 0;
  
  while (current_cluster >= 2 && current_cluster < 0xFFF8) {
    int next_cluster = fat16_get_fat_entry(disk, current_cluster);
    if (next_cluster < 0) {
      res = next_cluster;
      break;
    }
    
    // Free current cluster
    res = fat16_set_fat_entry(disk, current_cluster, VIOS_FAT16_UNUSED);
    if (res < 0) {
      break;
    }
    
    current_cluster = next_cluster;
  }
  
  return res;
}

/**
 * Update directory entry on disk
 */
static int fat16_update_directory_entry(struct disk *disk, struct fat_directory_item *item, int entry_sector, int entry_offset) {
  struct fat_private *private = disk->fs_private;
  int position = entry_sector * disk->sector_size + entry_offset;
  
  int res = diskstreamer_seek(private->directory_stream, position);
  if (res < 0) {
    return res;
  }
  
  res = diskstreamer_write(private->directory_stream, item, sizeof(struct fat_directory_item));
  return res;
}

/**
 * Write data to a file's cluster chain
 */
static int fat16_write_internal(struct disk *disk, int starting_cluster, uint32_t offset, const void *ptr, uint32_t total) {
  struct fat_private *private = disk->fs_private;
  struct fat_header *header = &private->header.primary_header;
  int cluster_size = header->sectors_per_cluster * disk->sector_size;
  
  // Find the cluster containing the offset
  int cluster = fat16_get_cluster_for_offset(disk, starting_cluster, offset);
  if (cluster < 0) {
    return cluster;
  }
  
  int offset_in_cluster = offset % cluster_size;
  int written = 0;
  int res = 0;
  
  while (written < (int)total) {
    int cluster_sector = fat16_cluster_to_sector(private, cluster);
    int cluster_position = cluster_sector * disk->sector_size + offset_in_cluster;
    
    int to_write = cluster_size - offset_in_cluster;
    if (to_write > (int)total - written) {
      to_write = (int)total - written;
    }
    
    res = diskstreamer_seek(private->cluster_read_stream, cluster_position);
    if (res < 0) {
      goto out;
    }
    
    res = diskstreamer_write(private->cluster_read_stream, (const char *)ptr + written, to_write);
    if (res < 0) {
      goto out;
    }
    
    written += to_write;
    offset_in_cluster = 0;  // Subsequent clusters start from beginning
    
    if (written < (int)total) {
      // Need next cluster
      int next_cluster = fat16_get_fat_entry(disk, cluster);
      if (next_cluster >= 0xFFF8) {
        // End of chain, need to allocate
        next_cluster = fat16_allocate_cluster(disk, cluster);
        if (next_cluster < 0) {
          res = next_cluster;
          goto out;
        }
      }
      cluster = next_cluster;
    }
  }
  
  res = written;
  
out:
  return res;
}

/**
 * Extend file size and update directory entry
 */
static int fat16_extend_file(struct disk *disk, struct fat_file_descriptor *desc, uint32_t new_size) {
  struct fat_directory_item *item = desc->item->item;
  uint32_t old_size = item->filesize;
  
  if (new_size <= old_size) {
    return 0;  // No extension needed
  }
  
  kernel_debug_log("[fat16_extend_file] extending from ");
  kernel_debug_log(itoa(old_size));
  kernel_debug_log(" to ");
  kernel_debug_log(itoa(new_size));
  kernel_debug_log(" bytes\n");
  
  // Update file size in directory entry
  item->filesize = new_size;
  
  // Write updated directory entry back to disk
  struct fat_private *private = disk->fs_private;
  int position = desc->item->directory_sector * disk->sector_size + desc->item->directory_offset;
  diskstreamer_seek(private->directory_stream, position);
  diskstreamer_write(private->directory_stream, item, sizeof(struct fat_directory_item));
  
  kernel_debug_log("[fat16_extend_file] directory entry written to disk\n");
  
  return 0;
}

/**
 * Main FAT16 write function
 */
int fat16_write(struct disk *disk, void *descriptor, const void *ptr, uint32_t size, uint32_t nmemb) {
  int res = 0;
  struct fat_file_descriptor *desc = descriptor;
  struct fat_item *desc_item = desc->item;
  
  if (desc_item->type != FAT_ITEM_TYPE_FILE) {
    res = -EINVARG;
    goto out;
  }
  
  struct fat_directory_item *item = desc_item->item;
  uint32_t total_to_write = size * nmemb;
  
  // Check if file is read-only
  if (item->attribute & FAT_FILE_READ_ONLY) {
    res = -ERDONLY;
    goto out;
  }
  
  // Get starting cluster
  uint32_t starting_cluster = fat16_get_first_cluster(item);
  
  // If file is empty, allocate first cluster
  if (starting_cluster == 0) {
    starting_cluster = fat16_allocate_cluster(disk, -1);
    if (starting_cluster < 0) {
      res = starting_cluster;
      goto out;
    }
    item->low_16_bits_first_cluster = starting_cluster & 0xFFFF;
    item->high_16_bits_first_cluster = (starting_cluster >> 16) & 0xFFFF;
  }
  
  // Write the data
  res = fat16_write_internal(disk, starting_cluster, desc->pos, ptr, total_to_write);
  if (res < 0) {
    goto out;
  }
  
  // Update file position
  desc->pos += res;
  
  // Update file size if we wrote past the end
  if (desc->pos > item->filesize) {
    fat16_extend_file(disk, desc, desc->pos);
  }
  
  // Return number of items written
  if (size > 0) {
    res = res / size;
  }
  
out:
  return res;
}

/**
 * Truncate file to specified size
 */
int fat16_truncate(struct disk *disk, void *descriptor, uint32_t size) {
  struct fat_file_descriptor *desc = descriptor;
  struct fat_item *desc_item = desc->item;
  
  if (desc_item->type != FAT_ITEM_TYPE_FILE) {
    return -EINVARG;
  }
  
  struct fat_directory_item *item = desc_item->item;
  
  if (item->attribute & FAT_FILE_READ_ONLY) {
    return -ERDONLY;
  }
  
  uint32_t starting_cluster = fat16_get_first_cluster(item);
  struct fat_private *private = disk->fs_private;
  int cluster_size = private->header.primary_header.sectors_per_cluster * disk->sector_size;
  
  if (size == 0) {
    // Free all clusters
    fat16_free_cluster_chain(disk, starting_cluster);
    item->low_16_bits_first_cluster = 0;
    item->high_16_bits_first_cluster = 0;
    item->filesize = 0;
    desc->pos = 0;
    return 0;
  }
  
  // Calculate how many clusters we need
  uint32_t clusters_needed = (size + cluster_size - 1) / cluster_size;
  
  // Find the cluster at the truncation point
  int cluster = starting_cluster;
  for (uint32_t i = 1; i < clusters_needed && cluster >= 2; i++) {
    int next = fat16_get_fat_entry(disk, cluster);
    if (next < 0) {
      return next;
    }
    cluster = next;
  }
  
  // Free everything after this cluster
  if (cluster >= 2) {
    int next_cluster = fat16_get_fat_entry(disk, cluster);
    if (next_cluster >= 2 && next_cluster < 0xFFF8) {
      fat16_free_cluster_chain(disk, next_cluster);
    }
    fat16_set_fat_entry(disk, cluster, 0xFFFF);  // Mark as end of chain
  }
  
  item->filesize = size;
  if (desc->pos > size) {
    desc->pos = size;
  }
  
  return 0;
}

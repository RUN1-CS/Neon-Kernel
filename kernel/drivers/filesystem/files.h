#ifndef FILES_H
#define FILES_H

#include "disk.h"

// Layout config
#define FS_START_LBA 64 // superblock location (keep clear of bootloader/kernel)
#define FS_MAX_FILES 64
#define FS_NAME_MAX 16 // incl. NUL -> 15 usable characters
#define FS_SECTOR_SIZE 512
#define FS_ENTRY_SIZE 32
#define FS_TABLE_SECTORS ((FS_MAX_FILES * FS_ENTRY_SIZE) / FS_SECTOR_SIZE)

// Return codes
#define FS_OK 0
#define FS_ERR_NOT_MOUNTED -1
#define FS_ERR_NOT_FOUND -2
#define FS_ERR_NO_SPACE -3
#define FS_ERR_TABLE_FULL -4
#define FS_ERR_BAD_NAME -5
#define FS_ERR_BAD_MAGIC -6

typedef void (*fs_list_cb)(const char *name, uint32_t size);

int fs_format(
    uint32_t total_sectors); // Wipes the disk and creates a new filesystem
int fs_mount(void);
int fs_exists(const char *name);
int fs_size(const char *name); // Returns the size of the file, or <0 on error
int fs_write(const char *name, const void *data,
             uint32_t size); // Create or overwrite a file
int fs_read(const char *name, void *buf, uint32_t max_len); // bytes read, or <0
int fs_delete(const char *name);
int fs_list(fs_list_cb cb); // number of files

#endif /* FILES_H */
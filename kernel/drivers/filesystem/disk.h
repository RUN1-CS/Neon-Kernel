#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define SECTOR_SIZE 512
#define FS_MAGIC    0x53555052

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t block_size;
    uint64_t total_blocks;
    char     volume_label[32];
} FilesystemHeader;

#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW      0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HIGH     0x1F5
#define ATA_PRIMARY_DRIVE_SEL    0x1F6
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_STATUS       0x1F7

#define ATA_CMD_READ_SECTORS     0x20
#define ATA_STATUS_BSY           0x80
#define ATA_STATUS_DRQ           0x08
#define ATA_STATUS_STATUS        0x1F7

static void ata_wait_ready(void);

static void ata_wait_drq(void);

void ata_read_sector(uint32_t lba, uint8_t *buffer);

void ata_write_sector(uint32_t lba, const uint8_t *buffer);

uint64_t kfind_filesystem_header(uint32_t max_lba_to_search);

#endif // DISK_H
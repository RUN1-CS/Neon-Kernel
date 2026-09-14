#ifndef DISK_C
#define DISK_C

#include "disk.h"
#include "../io.h"

static void ata_wait_ready(void) {
    while (port_byte_in(ATA_STATUS_STATUS) & ATA_STATUS_BSY);
}

// Wait until data buffer is ready to read
static void ata_wait_drq(void) {
    while (!(port_byte_in(ATA_STATUS_STATUS) & ATA_STATUS_DRQ));
}

// Read 1 LBA sector (512 bytes) using 28-bit LBA PIO mode
void ata_read_sector(uint32_t lba, uint8_t *buffer) {
    ata_wait_ready();

    port_byte_out(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    port_byte_out(ATA_PRIMARY_SECTOR_COUNT, 1);
    port_byte_out(ATA_PRIMARY_LBA_LOW,  (uint8_t)lba);
    port_byte_out(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    port_byte_out(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PRIMARY_COMMAND,  ATA_CMD_READ_SECTORS);

    ata_wait_drq();

    // Transfer 256 16-bit words (512 bytes total) from data port
    uint16_t *ptr = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = port_word_in(ATA_PRIMARY_DATA);
    }
}

// Write 1 LBA sector (512 bytes) using 28-bit LBA PIO mode
void ata_write_sector(uint32_t lba, const uint8_t *buffer) {
    ata_wait_ready();

    port_byte_out(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    port_byte_out(ATA_PRIMARY_SECTOR_COUNT, 1);
    port_byte_out(ATA_PRIMARY_LBA_LOW,  (uint8_t)lba);
    port_byte_out(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    port_byte_out(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PRIMARY_COMMAND,  ATA_CMD_READ_SECTORS);

    ata_wait_drq();

    // Transfer 256 16-bit words (512 bytes total) to data port
    const uint16_t *ptr = (const uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        port_word_out(ATA_PRIMARY_DATA, ptr[i]);
    }
}

#endif // DISK_C
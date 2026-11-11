#ifndef FILES_C
#define FILES_C

#include "files.h"
#include "disk.h"

/*
 * SimpleFS: flat directory, contiguous allocation.
 *
 *   LBA FS_START_LBA                         superblock
 *   LBA FS_START_LBA+1 .. +FS_TABLE_SECTORS  file table (FS_MAX_FILES entries)
 *   LBA after that                           file data (each file = one
 * contiguous extent)
 *
 * The file table is cached in RAM and written back after every change.
 */

#define SIMPLEFS_MAGIC 0x53465331u /* "SFS1" */
#define FS_VERSION 1u

#define FS_TABLE_LBA (FS_START_LBA + 1)
#define FS_DATA_LBA (FS_START_LBA + 1 + FS_TABLE_SECTORS)

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint32_t total_sectors; /* sectors available to the FS, counted from LBA 0
                             (end = this LBA) */
  uint32_t reserved[125];
} __attribute__((packed)) fs_super_t;

typedef struct {
  char name[FS_NAME_MAX];
  uint32_t start;   /* first LBA of data */
  uint32_t size;    /* bytes */
  uint32_t sectors; /* sectors allocated */
  uint32_t used;    /* 1 = in use */
} __attribute__((packed)) fs_entry_t;

static fs_entry_t table[FS_MAX_FILES] __attribute__((aligned(4)));
static uint8_t sector_buf[FS_SECTOR_SIZE] __attribute__((aligned(4)));
static uint32_t fs_total_sectors;
static int fs_mounted = 0;

//freestanding helpers
static void fs_memset(void *dst, uint8_t v, uint32_t n) {
  uint8_t *d = (uint8_t *)dst;
  while (n--)
    *d++ = v;
}

static void fs_memcpy(void *dst, const void *src, uint32_t n) {
  uint8_t *d = (uint8_t *)dst;
  const uint8_t *s = (const uint8_t *)src;
  while (n--)
    *d++ = *s++;
}

static uint32_t fs_strlen(const char *s) {
  uint32_t n = 0;
  while (s[n])
    n++;
  return n;
}

static int fs_streq(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

// Internal helpers
static int name_valid(const char *name) {
  if (!name)
    return 0;
  uint32_t len = fs_strlen(name);
  return len > 0 && len < FS_NAME_MAX;
}

static int find_entry(const char *name) {
  for (int i = 0; i < FS_MAX_FILES; i++) {
    if (table[i].used && fs_streq(table[i].name, name))
      return i;
  }
  return -1;
}

static int find_free_slot(void) {
  for (int i = 0; i < FS_MAX_FILES; i++) {
    if (!table[i].used)
      return i;
  }
  return -1;
}

static void flush_table(void) {
  for (uint32_t i = 0; i < FS_TABLE_SECTORS; i++) {
    ata_write_sector(FS_TABLE_LBA + i,
                     (const uint8_t *)table + i * FS_SECTOR_SIZE);
  }
}

static void load_table(void) {
  for (uint32_t i = 0; i < FS_TABLE_SECTORS; i++) {
    ata_read_sector(FS_TABLE_LBA + i, (uint8_t *)table + i * FS_SECTOR_SIZE);
  }
}

// First-fit search for a contiguous run of `sectors` free sectors.
// Returns the start LBA, or 0 if nothing fits.
static uint32_t alloc_extent(uint32_t sectors) {
  uint32_t candidate = FS_DATA_LBA;
  int moved;

  do {
    moved = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
      if (!table[i].used || table[i].sectors == 0)
        continue;
      uint32_t s = table[i].start;
      uint32_t e = s + table[i].sectors;
      if (candidate < e && candidate + sectors > s) {
        candidate = e; /* overlaps: jump past this file and re-check */
        moved = 1;
      }
    }
  } while (moved);

  if (candidate + sectors > fs_total_sectors)
    return 0;
  return candidate;
}

// Public API
int fs_format(uint32_t total_sectors) {
  if (total_sectors <= FS_DATA_LBA)
    return FS_ERR_NO_SPACE;

  fs_super_t *sb = (fs_super_t *)sector_buf;
  fs_memset(sector_buf, 0, FS_SECTOR_SIZE);
  sb->magic = SIMPLEFS_MAGIC;
  sb->version = FS_VERSION;
  sb->total_sectors = total_sectors;
  ata_write_sector(FS_START_LBA, sector_buf);

  fs_memset(table, 0, sizeof(table));
  flush_table();

  return fs_mount();
}

int fs_mount(void) {
  ata_read_sector(FS_START_LBA, sector_buf);
  fs_super_t *sb = (fs_super_t *)sector_buf;

  if (sb->magic != SIMPLEFS_MAGIC || sb->version != FS_VERSION) {
    fs_mounted = 0;
    return FS_ERR_BAD_MAGIC;
  }

  fs_total_sectors = sb->total_sectors;
  load_table();
  fs_mounted = 1;
  return FS_OK;
}

int fs_exists(const char *name) {
  if (!fs_mounted || !name_valid(name))
    return 0;
  return find_entry(name) >= 0;
}

int fs_size(const char *name) {
  if (!fs_mounted)
    return FS_ERR_NOT_MOUNTED;
  if (!name_valid(name))
    return FS_ERR_BAD_NAME;
  int i = find_entry(name);
  if (i < 0)
    return FS_ERR_NOT_FOUND;
  return (int)table[i].size;
}

int fs_write(const char *name, const void *data, uint32_t size) {
  if (!fs_mounted)
    return FS_ERR_NOT_MOUNTED;
  if (!name_valid(name))
    return FS_ERR_BAD_NAME;

  uint32_t need = (size + FS_SECTOR_SIZE - 1) / FS_SECTOR_SIZE;
  int idx = find_entry(name);
  uint32_t start = 0;

  if (idx >= 0 && need <= table[idx].sectors) {
    // Fits in the existing extent: reuse it.
    start = table[idx].start;
    need = table[idx].sectors;
  } else {
    fs_entry_t old;
    int had_old = (idx >= 0);

    if (had_old) {
      old = table[idx];
      table[idx].used = 0; // free old extent so it can be reused
    } else {
      idx = find_free_slot();
      if (idx < 0)
        return FS_ERR_TABLE_FULL;
    }

    if (need > 0) {
      start = alloc_extent(need);
      if (start == 0) {
        if (had_old)
          table[idx] = old; // roll back
        return FS_ERR_NO_SPACE;
      }
    }
  }

  // Write data, zero-padding the final sector.
  const uint8_t *src = (const uint8_t *)data;
  uint32_t remaining = size;
  for (uint32_t s = 0; s < need; s++) {
    uint32_t chunk = remaining > FS_SECTOR_SIZE ? FS_SECTOR_SIZE : remaining;
    fs_memset(sector_buf, 0, FS_SECTOR_SIZE);
    if (chunk)
      fs_memcpy(sector_buf, src, chunk);
    ata_write_sector(start + s, sector_buf);
    src += chunk;
    remaining -= chunk;
  }

  fs_memset(table[idx].name, 0, FS_NAME_MAX);
  fs_memcpy(table[idx].name, name, fs_strlen(name));
  table[idx].start = start;
  table[idx].size = size;
  table[idx].sectors = need;
  table[idx].used = 1;

  flush_table();
  return FS_OK;
}

int fs_read(const char *name, void *buf, uint32_t max_len) {
  if (!fs_mounted)
    return FS_ERR_NOT_MOUNTED;
  if (!name_valid(name))
    return FS_ERR_BAD_NAME;

  int idx = find_entry(name);
  if (idx < 0)
    return FS_ERR_NOT_FOUND;

  uint32_t to_read = table[idx].size < max_len ? table[idx].size : max_len;
  uint8_t *dst = (uint8_t *)buf;
  uint32_t done = 0;

  for (uint32_t s = 0; done < to_read; s++) {
    ata_read_sector(table[idx].start + s, sector_buf);
    uint32_t chunk = to_read - done;
    if (chunk > FS_SECTOR_SIZE)
      chunk = FS_SECTOR_SIZE;
    fs_memcpy(dst + done, sector_buf, chunk);
    done += chunk;
  }
  return (int)done;
}

int fs_delete(const char *name) {
  if (!fs_mounted)
    return FS_ERR_NOT_MOUNTED;
  if (!name_valid(name))
    return FS_ERR_BAD_NAME;

  int idx = find_entry(name);
  if (idx < 0)
    return FS_ERR_NOT_FOUND;

  fs_memset(&table[idx], 0, sizeof(fs_entry_t));
  flush_table();
  return FS_OK;
}

int fs_list(fs_list_cb cb) {
  if (!fs_mounted)
    return FS_ERR_NOT_MOUNTED;

  int count = 0;
  for (int i = 0; i < FS_MAX_FILES; i++) {
    if (!table[i].used)
      continue;
    if (cb)
      cb(table[i].name, table[i].size);
    count++;
  }
  return count;
}

#endif // FILES_C
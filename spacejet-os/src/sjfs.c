/*
 * SpaceJet File System — RAM-backed flat filesystem.
 * The SjfsStore struct sits at SJFS_BASE_ADDR (0x0F000000).
 * QEMU versatilepb has 256 MB RAM (0x00000000–0x0FFFFFFF),
 * so this is valid and won't collide with our code/stack.
 *
 * WARNING: data is in volatile RAM.  For real hardware, back
 * this with SPI-NOR flash (e.g. W25Q128) and add wear-levelling.
 */
#include "sjfs.h"
#include "kstring.h"
#include "kprintf.h"

void sjfs_init(void) {
    SjfsStore *fs = sjfs_store;
    if (fs->magic == SJFS_MAGIC && fs->version == 1) {
        kprintf("  [SJFS] Existing filesystem mounted (%u files)\n",
                (unsigned)SJFS_MAX_FILES);
        return;   /* already initialised (soft-reboot survives) */
    }
    /* First ever boot: format */
    kmemset(fs, 0, sizeof(SjfsStore));
    fs->magic   = SJFS_MAGIC;
    fs->version = 1;
    kprintf("  [SJFS] Formatted fresh filesystem at 0x%08x\n",
            (unsigned)SJFS_BASE_ADDR);
}

int sjfs_open(const char *path) {
    SjfsStore *fs = sjfs_store;
    for (int i = 0; i < SJFS_MAX_FILES; i++)
        if (fs->entries[i].used && kstrcmp(fs->entries[i].path, path) == 0)
            return i;
    return -1;
}

int sjfs_create(const char *path) {
    SjfsStore *fs = sjfs_store;
    for (int i = 0; i < SJFS_MAX_FILES; i++) {
        if (!fs->entries[i].used) {
            kstrncpy(fs->entries[i].path, path, SJFS_PATH_LEN - 1);
            fs->entries[i].write_pos = 0;
            fs->entries[i].used      = true;
            kmemset(fs->data[i], 0, SJFS_FILE_BYTES);
            return i;
        }
    }
    return -1;   /* filesystem full */
}

int sjfs_open_or_create(const char *path) {
    int idx = sjfs_open(path);
    return (idx >= 0) ? idx : sjfs_create(path);
}

int sjfs_write(int idx, const uint8_t *data, uint32_t len) {
    if (idx < 0 || idx >= SJFS_MAX_FILES) return -1;
    SjfsStore *fs  = sjfs_store;
    SjfsEntry *ent = &fs->entries[idx];
    uint32_t space = SJFS_FILE_BYTES - ent->write_pos;
    if (len > space) len = space;
    kmemcpy(&fs->data[idx][ent->write_pos], data, len);
    ent->write_pos += len;
    return (int)len;
}

int sjfs_append_str(int idx, const char *s) {
    return sjfs_write(idx, (const uint8_t *)s, (uint32_t)kstrlen(s));
}

int sjfs_read(int idx, uint8_t *out, uint32_t maxlen, uint32_t *got) {
    if (idx < 0 || idx >= SJFS_MAX_FILES) return -1;
    SjfsStore *fs = sjfs_store;
    uint32_t n = fs->entries[idx].write_pos;
    if (n > maxlen) n = maxlen;
    kmemcpy(out, fs->data[idx], n);
    if (got) *got = n;
    return 0;
}

int sjfs_rename(const char *old_path, const char *new_path) {
    int idx = sjfs_open(old_path);
    if (idx < 0) return -1;
    kstrncpy(sjfs_store->entries[idx].path, new_path, SJFS_PATH_LEN - 1);
    return 0;
}

void sjfs_list(void) {
    SjfsStore *fs = sjfs_store;
    kprintf("  SJFS contents:\n");
    for (int i = 0; i < SJFS_MAX_FILES; i++) {
        if (fs->entries[i].used)
            kprintf("    [%2d] %u B  %s\n", i,
                    (unsigned)fs->entries[i].write_pos,
                    fs->entries[i].path);
    }
}

#ifndef SJFS_H
#define SJFS_H
#include <stdint.h>
#include <stdbool.h>

#define SJFS_MAX_FILES  24
#define SJFS_PATH_LEN   72      /* enough for /_logs/dd_mm_yyyy/hh.mm_hh.mm/logs */
#define SJFS_FILE_BYTES 4096    /* 4 KB per file — sufficient for log/key files   */
#define SJFS_MAGIC      0x534A4653U   /* "SJFS" */

typedef struct {
    char     path[SJFS_PATH_LEN];
    uint32_t write_pos;         /* bytes written so far  */
    bool     used;
    uint8_t  _pad[3];
} SjfsEntry;

typedef struct {
    uint32_t  magic;
    uint32_t  version;
    SjfsEntry entries[SJFS_MAX_FILES];
    /* File data follows in the data[] array below */
    uint8_t   data[SJFS_MAX_FILES][SJFS_FILE_BYTES];
} SjfsStore;

/* The store lives at a fixed high-RAM address */
#define SJFS_BASE_ADDR  0x0F000000U
#define sjfs_store      ((SjfsStore *)(SJFS_BASE_ADDR))

void  sjfs_init(void);
int   sjfs_open(const char *path);          /* returns index, -1 if not found */
int   sjfs_create(const char *path);        /* creates new, returns index      */
int   sjfs_open_or_create(const char *path);
int   sjfs_write(int idx, const uint8_t *data, uint32_t len);
int   sjfs_append_str(int idx, const char *s);
int   sjfs_read(int idx, uint8_t *out, uint32_t maxlen, uint32_t *got);
int   sjfs_rename(const char *old_path, const char *new_path);
void  sjfs_list(void);                      /* debug: list all files via kprintf */
#endif

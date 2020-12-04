#ifndef GOG_HEADER_H
#define GOG_HEADER_H

#include "gogio.h"

typedef struct {
    char *filename;
    uint32_t location;
    int is_temporary;
} gog_header_file_entry;

typedef struct {
    uint32_t chunk_first_slice;
    uint32_t chunk_last_slice;
    uint32_t chunk_offset;

    uint64_t file_offset;
    uint64_t file_size;
    uint64_t chunk_size;

    uint8_t sha1_checksum[20];
    uint8_t compressed;
    uint8_t exe_optimized;
} gog_header_data_entry;

typedef struct {
    struct {
        int major;
        int minor;
        int patch;
        int unicode;
    } version;
    uint32_t num_files;
    gog_header_file_entry *files;
    uint32_t num_data_entries;
    gog_header_data_entry *data_entries;
} gog_header_info;

int gog_header_load(io_source *src, gog_header_info *info);

void gog_header_free(gog_header_info *info);

#endif

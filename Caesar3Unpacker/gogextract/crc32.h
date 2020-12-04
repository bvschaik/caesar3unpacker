#ifndef GOG_CRC32_H
#define GOG_CRC32_H

#include <stdint.h>

#include "gogio.h"

typedef struct {
    uint32_t checksum;
} crc32_t;

void crc32_init(crc32_t *crc);

void crc32_update(crc32_t *crc, const void *data, int length);

uint32_t crc32_finish(crc32_t *crc);

io_source *crc32_io_attach(io_source *base);

uint32_t crc32_io_detach(io_source *source);

#endif

#ifndef GOG_SHA1_H
#define GOG_SHA1_H

#include <stdint.h>

#include "gogio.h"

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} sha1_t;

void sha1_init(sha1_t* context);
void sha1_update(sha1_t* context, const uint8_t* data, uint32_t len);
void sha1_finish(sha1_t* context, uint8_t digest[20]);

io_source *sha1_io_attach(io_source *base, uint8_t digest[20]);

#endif

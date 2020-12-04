#ifndef GOG_EXE_H
#define GOG_EXE_H

#include "gogio.h"

#include <stdint.h>

typedef struct {
    uint32_t header_offset;
    uint32_t data_offset;
} gog_exe_offsets;

int gog_exe_get_offsets(io_source *src, gog_exe_offsets *offsets);

#endif

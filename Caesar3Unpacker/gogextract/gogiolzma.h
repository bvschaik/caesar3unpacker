#ifndef GOG_IO_LZMA_H
#define GOG_IO_LZMA_H

#include "gogio.h"

enum {
    GOG_IO_LZMA1 = 1,
    GOG_IO_LZMA2 = 2
};

io_source *lzma_io_open(int lzma_version, io_source *base);

#endif

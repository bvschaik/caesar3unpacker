#ifndef GOG_IO_FILE_H
#define GOG_IO_FILE_H

#include "gogio.h"

#include <stdio.h>

io_source *file_io_open(FILE *fp, int keep_open);

#endif

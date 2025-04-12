#ifndef GOG_FILE_H
#define GOG_FILE_H

#include "gogheader.h"
#include "gogio.h"

int gog_file_save(io_source *src, gog_header_data_entry *entry, const char *filepath, int append, int output_compressed);

#endif

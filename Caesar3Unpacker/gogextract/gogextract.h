#ifndef GOGEXTRACT_H
#define GOGEXTRACT_H

#include <stddef.h>

typedef struct _gogextract gogextract;

gogextract* gogextract_open(const char *filename);

void gogextract_close(gogextract *g);

int gogextract_get_num_files(gogextract *g);

const char *gogextract_get_file_name(gogextract *g, int id);

size_t gogextract_get_file_size(gogextract *g, int id);

int gogextract_save_file(gogextract *g, int id, const char *filepath);

const char *gogextract_error(void);

#endif /* GOGEXTRACT_H */

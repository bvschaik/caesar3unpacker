#ifndef gogextract_h
#define gogextract_h

#include <stdio.h>

typedef struct {
    int id;
} gogextract;

gogextract *gogextract_open(const char *filename);

void gogextract_close(gogextract *g);

const char *gogextract_error(void);

int gogextract_get_num_files(gogextract *g);

const char *gogextract_get_file_name(gogextract *g, int index);

int gogextract_save_file(gogextract *g, int index, const char *filepath);

#endif /* gogextract_h */

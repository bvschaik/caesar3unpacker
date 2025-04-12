#include "gogextract.h"

#include "gogerror.h"
#include "gogexe.h"
#include "gogfile.h"
#include "gogheader.h"
#include "gogioblock.h"
#include "gogiofile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _gogextract {
    io_source *src;
    gog_exe_offsets offsets;
    gog_header_info info;
    int *file_index;
    int num_files;
};

static int prepare_files(gogextract *g)
{
    g->file_index = calloc(g->info.num_files, sizeof(int));
    if (!g->file_index) {
        gog_set_error("Unable to allocate memory");
        return 0;
    }
    int file_id = 0;
    for (int i = 0; i < g->info.num_files; i++) {
        gog_header_file_entry *file = &g->info.files[i];
        if (file->is_temporary || !file->filename || !file->filename[0]) {
            continue;
        }
        g->file_index[file_id++] = i;
    }
    g->num_files = file_id;
    return 1;
}

gogextract* gogextract_open(const char *filename)
{
    gogextract *g = (gogextract *)malloc(sizeof(gogextract));
    if (!g) {
        gog_set_error("Unable to allocate memory");
        return 0;
    }
    memset(g, 0, sizeof(gogextract));
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        gogextract_close(g);
        gog_set_error("Unable to open file %s", filename);
        return 0;
    }
    g->src = file_io_open(fp, 0);

    if (!gog_exe_get_offsets(g->src, &g->offsets)) {
        gogextract_close(g);
        gog_set_error("The file does not appear to be an InnoSetup executable");
        return 0;
    }

    io_set(g->src, g->offsets.header_offset);
    if (!gog_header_load(g->src, &g->info)) {
        gogextract_close(g);
        return 0;
    }

    if (!prepare_files(g)) {
        gogextract_close(g);
        return 0;
    }
    return g;
}

void gogextract_close(gogextract *g)
{
    if (g) {
        io_close(g->src);
        gog_header_free(&g->info);
        free(g);
    }
}

int gogextract_get_num_files(gogextract *g)
{
    return g->num_files;
}

const char *gogextract_get_file_name(gogextract *g, int id)
{
    if (id < 0 || id >= g->num_files) {
        return 0;
    }
    return g->info.files[g->file_index[id]].filename;
}

int gogextract_save_file(gogextract *g, int id, const char *filepath)
{
    if (id < 0 || id >= g->num_files) {
        return 0;
    }
    gog_header_file_entry *file = &g->info.files[g->file_index[id]];
    for (int i = 0; i < file->num_locations; i++) {
        int location = file->locations[i];
        if (location < 0 || location >= g->info.num_data_entries) {
            return 0;
        }
        gog_header_data_entry *entry = &g->info.data_entries[location];
        if (!io_set(g->src, g->offsets.data_offset)) {
            gog_set_error("Cannot seek to %d", g->offsets.data_offset);
            return 0;
        }
        if (!gog_file_save(g->src, entry, filepath, i > 0)) {
            return 0;
        }
    }
    return 1;
}

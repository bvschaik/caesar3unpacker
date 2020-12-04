#include "gogiofile.h"

typedef struct {
    FILE *fp;
    int keep_open;
} fp_io_data;

static int fp_read_bytes(void *source_data, void *buffer, int max_size)
{
    FILE *fp = ((fp_io_data *)source_data)->fp;
    return fread(buffer, 1, max_size, fp);
}

static int fp_seek_to(void *source_data, long offset, int whence)
{
    FILE *fp = ((fp_io_data *)source_data)->fp;
    return fseek(fp, offset, whence) == 0;
}

static int fp_has_error(void *source_data)
{
    FILE *fp = ((fp_io_data *)source_data)->fp;
    return ferror(fp);
}

static long fp_get_pos(void *source_data)
{
    FILE *fp = ((fp_io_data *)source_data)->fp;
    return ftell(fp);
}

static void fp_close(void *source_data)
{
    fp_io_data *data = (fp_io_data *)source_data;
    if (!data->keep_open) {
        fclose(data->fp);
    }
}

io_source *file_io_open(FILE *fp, int keep_open)
{
    io_source *s = io_create(sizeof(fp_io_data));
    if (s) {
        fp_io_data *data = (fp_io_data *) s->data;
        data->fp = fp;
        data->keep_open = keep_open;
        s->read_bytes = fp_read_bytes;
        s->seek_to = fp_seek_to;
        s->get_pos = fp_get_pos;
        s->has_error = fp_has_error;
        s->close = fp_close;
    }
    return s;
}

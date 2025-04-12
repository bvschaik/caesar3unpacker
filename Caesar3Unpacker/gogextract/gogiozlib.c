#include "gogiozlib.h"

#include <string.h>
#include <zlib.h>

#define CHUNK 16384

typedef struct {
    io_source *base;
    int in_bytes_left;
    int out_offset;
    int out_max;
    int has_error;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
} zlib_io_data;

static int init(zlib_io_data *data, int length)
{
    data->in_bytes_left = length;
    data->out_offset = 0;
    data->out_max = 0;
    data->has_error = 0;

    data->strm.zalloc = Z_NULL;
    data->strm.zfree = Z_NULL;
    data->strm.opaque = Z_NULL;
    data->strm.avail_in = 0;
    data->strm.next_in = Z_NULL;
    return inflateInit(&data->strm) == Z_OK;
}

static int read_chunk(zlib_io_data *data)
{
    int to_read = data->in_bytes_left < CHUNK ? data->in_bytes_left : CHUNK;
    data->strm.avail_in = io_read_raw(data->base, data->in, to_read);
    data->in_bytes_left -= to_read;
    if (!data->strm.avail_in) {
        return 0;
    }
    data->strm.next_in = data->in;
    return 1;
}

static int decompress_chunk(zlib_io_data *data)
{
    data->strm.avail_out = CHUNK;
    data->strm.next_out = data->out;
    int ret = inflate(&data->strm, Z_NO_FLUSH);
    if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
        data->out_offset = 0;
        data->out_max = 0;
        data->has_error = 1;
        return 0;
    }

    data->out_offset = 0;
    data->out_max = CHUNK - data->strm.avail_out;
    return 1;
}

static int read_bytes(void *source_data, void *buffer, int max_size)
{
    zlib_io_data *data = (zlib_io_data *) source_data;
    if (data->has_error) {
        return 0;
    }

    int buffer_offset = 0;
    do {
        // Copy from existing out buffer
        if (data->out_offset < data->out_max) {
            int copy_size = data->out_max - data->out_offset < max_size - buffer_offset ? data->out_max - data->out_offset : max_size - buffer_offset;
            memcpy(&buffer[buffer_offset], &data->out[data->out_offset], copy_size);
            data->out_offset += copy_size;
            buffer_offset += copy_size;
        }
        if (buffer_offset == max_size) {
            return max_size;
        }

        // Decompress new chunk from existing input
        if (!decompress_chunk(data)) {
            return buffer_offset;
        }
        if (data->out_max == 0) {
            // Read new chunk from input
            if (!read_chunk(data)) {
                return buffer_offset;
            }
        }
    } while (buffer_offset < max_size);
    return buffer_offset;
}

static int has_error(void *source_data)
{
    zlib_io_data *data = (zlib_io_data *) source_data;
    return data->has_error || data->base->has_error(data->base->data);
}

static void zlib_io_close(void *source_data)
{
    zlib_io_data *data = (zlib_io_data *)source_data;
    inflateEnd(&data->strm);
    io_close(data->base);
}

io_source *zlib_io_open(int length, io_source *base)
{
    io_source *s = io_create(sizeof(zlib_io_data));
    if (s) {
        zlib_io_data *data = (zlib_io_data *) s->data;
        data->base = base;
        if (!init(data, length)) {
            io_close(s);
            return NULL;
        }
        s->read_bytes = read_bytes;
        s->has_error = has_error;
        s->close = zlib_io_close;
    }
    return s;
}

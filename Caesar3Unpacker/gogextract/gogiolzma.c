#include "gogiolzma.h"

#include "lzma/lzma.h"

#define BUFFER_SIZE 4096

typedef struct {
    io_source *base;
    lzma_stream stream;
    int has_error;
    int at_end;
    uint8_t input_buffer[BUFFER_SIZE];
} lzma_io_data;

static int fill_input_buffer(lzma_io_data *data)
{
    if (data->stream.avail_in <= 0) {
        data->stream.next_in = data->input_buffer;
        data->stream.avail_in = io_read_raw(data->base, data->input_buffer, BUFFER_SIZE);
    }
    return data->stream.avail_in > 0;
}

static int read_bytes(void *source_data, void *buffer, int max_size)
{
    lzma_io_data *data = (lzma_io_data *)source_data;
    if (data->at_end) {
        return 0;
    }

    data->stream.next_out = (uint8_t *)buffer;
    data->stream.avail_out = max_size;

    lzma_ret ret;
    do {
        if (!fill_input_buffer(data)) {
            break;
        }

        ret = lzma_code(&data->stream, LZMA_RUN);
    } while (ret == LZMA_OK && data->stream.avail_out);
    if (ret == LZMA_STREAM_END) {
        data->at_end = 1;
    } else if (ret != LZMA_OK) {
        data->has_error = 1;
    }
    return max_size - data->stream.avail_out;
}

static int has_error(void *source_data)
{
    lzma_io_data *data = (lzma_io_data *)source_data;
    return data->has_error;
}

static void close(void *source_data)
{
    lzma_io_data *data = (lzma_io_data *)source_data;
    lzma_end(&data->stream);
}

static int init(lzma_io_data *data, int version)
{
    lzma_options_lzma options = {0};

    if (version == GOG_IO_LZMA1) {
        uint8_t props = io_read_u8(data->base);
        uint32_t dict_size = io_read_u32(data->base);

        options.pb = props / (9 * 5);
        options.lp = (props % (9 * 5)) / 9;
        options.lc = props % 9;
        options.dict_size = dict_size;
    } else if (version == GOG_IO_LZMA2) {
        uint8_t props = io_read_u8(data->base);
        if (props > 40) {
            return 0;
        }
        if (props == 40) {
            options.dict_size = 0xffffffff;
        } else {
            options.dict_size = (uint32_t) (2 | (props & 1)) << ((props) / 2 + 11);
        }
    } else {
        return 0;
    }

    lzma_stream *stream = &data->stream;
    lzma_stream tmp = LZMA_STREAM_INIT;
    *stream = tmp;
    stream->allocator = NULL;

    options.preset_dict = NULL;
    const lzma_filter filters[2] = {
        {version == 1 ? LZMA_FILTER_LZMA1 : LZMA_FILTER_LZMA2,  &options},
        {LZMA_VLI_UNKNOWN, NULL}
    };

    lzma_ret ret = lzma_raw_decoder(stream, filters);
    return ret == LZMA_OK;
}

io_source *lzma_io_open(int version, io_source *base)
{
    if (version != GOG_IO_LZMA1 && version != GOG_IO_LZMA2) {
        return 0;
    }
    io_source *s = io_create(sizeof(lzma_io_data));
    if (s) {
        lzma_io_data *data = (lzma_io_data *) s->data;
        data->base = base;
        data->has_error = 0;
        data->at_end = 0;
        s->read_bytes = read_bytes;
        s->has_error = has_error;
        s->close = close;
        if (!init(data, version)) {
            io_close(s);
            s = 0;
        }
    }
    return s;
}

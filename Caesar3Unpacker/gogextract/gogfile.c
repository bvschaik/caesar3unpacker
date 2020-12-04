#include "gogfile.h"

#include "gogerror.h"
#include "gogiolzma.h"
#include "sha1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8192

static const uint8_t EXPECTED_MAGIC[4] = {'z','l','b',0x1a};

#define EXEFILTER_BLOCK 0x10000

enum {
    EXE_SEEK,
    EXE_READ_ADDRESS,
    EXE_COPY_ADDRESS
};

typedef struct {
    io_source *base;
    int total_bytes;
    int state;
    uint8_t address[4];
    int address_offset;
    int address_max;
} exefilter_io_data;

static int is_block_boundary(int offset)
{
    int left = EXEFILTER_BLOCK - (offset % EXEFILTER_BLOCK);
    return left < 5;
}

static int exefilter_read_bytes(void *source_data, void *buffer, int length)
{
    exefilter_io_data *data = (exefilter_io_data *)source_data;
    uint8_t *bytes = (uint8_t *) buffer;
    int bytes_read = 0;
    if (data->state == EXE_COPY_ADDRESS) {
        bytes_read = data->address_max - data->address_offset;
        if (bytes_read > length) {
            bytes_read = length;
        }
    }
    if (bytes_read < length) {
        bytes_read += io_read_raw(data->base, &bytes[bytes_read], length - bytes_read);
    }
    for (int i = 0; i < bytes_read; i++) {
        switch (data->state) {
            case EXE_SEEK:
                if ((bytes[i] == 0xe8 || bytes[i] == 0xe9) && !is_block_boundary(data->total_bytes + i)) {
                    data->state = EXE_READ_ADDRESS;
                }
                break;
            case EXE_READ_ADDRESS:
                data->address_max = bytes_read < i + 4 ? bytes_read - i : 4;
                for (int n = 0; n < data->address_max; n++) {
                    data->address[n] = bytes[i+n];
                }
                if (data->address_max < 4) {
                    data->address_max += io_read_raw(data->base, &data->address[data->address_max], 4 - data->address_max);
                }
                if (data->address_max == 4 && (data->address[3] == 0x00 || data->address[3] == 0xff)) {
                    uint32_t rel = data->address[0] | (data->address[1] << 8) | (data->address[2] << 16);
                    rel -= (data->total_bytes + i + 4) & 0xffffff;
                    data->address[0] = rel & 0xff;
                    data->address[1] = (rel >> 8) & 0xff;
                    data->address[2] = (rel >> 16) & 0xff;
                    if (rel & 0x800000) {
                        data->address[3] = ~data->address[3];
                    }
                }
                data->state = EXE_COPY_ADDRESS;
                // intentional fall-through
            case EXE_COPY_ADDRESS:
                bytes[i] = data->address[data->address_offset++];
                if (data->address_offset >= data->address_max) {
                    data->address_offset = 0;
                    data->state = EXE_SEEK;
                }
                break;
        }
    }
    data->total_bytes += bytes_read;
    return bytes_read;
}

static int exefilter_has_error(void *source_data)
{
    exefilter_io_data *data = (exefilter_io_data *)source_data;
    return io_error(data->base);
}

static void exefilter_close(void *source_data)
{
    exefilter_io_data *data = (exefilter_io_data *)source_data;
    io_close(data->base);
}

static io_source *exefilter_io_open(io_source *base)
{
    io_source *s = io_create(sizeof(exefilter_io_data));
    if (s) {
        exefilter_io_data *data = (exefilter_io_data *) s->data;
        data->base = base;
        data->total_bytes = 0;
        s->read_bytes = exefilter_read_bytes;
        s->has_error = exefilter_has_error;
        s->close = exefilter_close;
    }
    return s;
}

static int write_file(io_source *src, FILE *fp, int length, uint8_t expected_checksum[20])
{
    uint8_t buffer[BUFFER_SIZE];
    io_source *sha = sha1_io_attach(src);
    do {
        int to_read = length < BUFFER_SIZE ? length : BUFFER_SIZE;
        int bytes_read = io_read_raw(sha, buffer, to_read);
        if (to_read != bytes_read) {
            gog_set_error("Unable to read data");
            return 0;
        }
        fwrite(buffer, 1, to_read, fp);
        length -= to_read;
    } while (length > 0);
    uint8_t checksum[20];
    sha1_io_detach(sha, checksum);
    if (memcmp(checksum, expected_checksum, 20) != 0) {
        gog_set_error("Checksum mismatch");
        return 0;
    }
    return 1;
}

int gog_file_save(io_source *src, gog_header_data_entry *entry, const char *filepath)
{
    io_skip(src, entry->chunk_offset);
    uint8_t magic[4];
    if (4 != io_read_raw(src, magic, 4) || memcmp(magic, EXPECTED_MAGIC, 4) != 0) {
        gog_set_error("Unable to read magic");
        return 0;
    }
    io_source *file_src = src;
    if (entry->compressed) {
        file_src = lzma_io_open(GOG_IO_LZMA2, src);
        if (!file_src) {
            gog_set_error("Unable to open compressed data stream");
            return 0;
        }
    }
    if (entry->exe_optimized) {
        file_src = exefilter_io_open(file_src);
    }
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        gog_set_error("Unable to open file '%s' for writing", filepath);
        if (entry->compressed) {
            io_close(file_src);
        }
        return 0;
    }
    int result = write_file(file_src, fp, entry->file_size, entry->sha1_checksum);
    fclose(fp);
    if (entry->compressed) {
        io_close(file_src);
    }
    return result;
}

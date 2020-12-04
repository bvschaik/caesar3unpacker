#include "gogiofile.h"

#include "crc32.h"
#include "gogiolzma.h"

#include <stdlib.h>
#include <string.h>

#define BLOCK_CHECKSUM_SIZE 4
#define BLOCK_DATA_SIZE 4096
#define BLOCK_FULL_SIZE (BLOCK_DATA_SIZE + BLOCK_CHECKSUM_SIZE)

typedef struct {
    io_source *base;
    uint8_t buffer[BLOCK_DATA_SIZE];
    uint32_t block_index;
    uint32_t block_length;
    uint32_t block_offset;
    uint32_t length;
} block_io_data;

static int read_block(block_io_data *data, uint32_t offset)
{
    data->block_offset = offset;
    data->block_index = 0;
    data->block_length = 0;

    if (offset > data->length) {
        return 0;
    }

    int length = data->length - offset;
    if (length > BLOCK_DATA_SIZE) {
        length = BLOCK_DATA_SIZE;
    }

    uint32_t expected_checksum = io_read_u32(data->base);
    io_source *crc32 = crc32_io_attach(data->base);
    io_read_raw(crc32, data->buffer, length);
    uint32_t actual_checksum = crc32_io_detach(crc32);

    if (actual_checksum != expected_checksum) {
        printf("Checksum mismatch on block at offset %d %x <> %x\n", offset, actual_checksum, expected_checksum);
        return 0;
    }
    data->block_length = length;
    return 1;
}

static int copy_bytes(block_io_data *data, uint8_t *buffer, int max_bytes)
{
    int available_bytes = data->block_length - data->block_index;
    if (available_bytes > max_bytes) {
        available_bytes = max_bytes;
    }
    if (available_bytes > 0) {
        memcpy(buffer, &data->buffer[data->block_index], available_bytes);
        data->block_index += available_bytes;
    }
    return available_bytes;
}

static int read_bytes(void *source_data, void *buffer, int max_size)
{
    block_io_data *data = (block_io_data *)source_data;
    uint8_t *dst = (uint8_t *)buffer;
    int bytes_to_copy = max_size;
    int total_bytes = 0;
    int copied_bytes = 0;
    do {
        copied_bytes = copy_bytes(data, dst, bytes_to_copy);
        if (copied_bytes < bytes_to_copy && read_block(data, data->block_offset + BLOCK_DATA_SIZE)) {
            copied_bytes += copy_bytes(data, &dst[copied_bytes], bytes_to_copy - copied_bytes);
        }
        dst += copied_bytes;
        total_bytes += copied_bytes;
        bytes_to_copy -= copied_bytes;
    } while (copied_bytes && total_bytes < max_size);
    return total_bytes;
}

static int has_error(void *source_data)
{
    block_io_data *data = (block_io_data *)source_data;
    return data->block_offset > data->length;
}

static io_source *init_block_source(io_source *s, io_source *base)
{
    block_io_data *data = (block_io_data *) s->data;
    data->base = base;
    data->block_offset = 0;
    uint32_t expected_checksum = io_read_u32(base);

    io_source *crc32 = crc32_io_attach(base);
    uint32_t total_length = io_read_u32(crc32);
    int full_blocks = total_length / BLOCK_FULL_SIZE;
    data->length = full_blocks * BLOCK_DATA_SIZE;
    if (total_length > full_blocks * BLOCK_FULL_SIZE) {
        data->length += total_length - full_blocks * BLOCK_FULL_SIZE - BLOCK_CHECKSUM_SIZE;
    }
    int compressed = io_read_u8(crc32);
    uint32_t actual_checksum = crc32_io_detach(crc32);

    if (actual_checksum != expected_checksum) {
        printf("Checksum mismatch %x <> %x\n", actual_checksum, expected_checksum);
        return 0;
    }
    if (!read_block(data, 0)) {
        printf("Unable to read first block\n");
        return 0;
    }
    if (compressed) {
        return lzma_io_open(GOG_IO_LZMA1, s);
    } else {
        return s;
    }
}

io_source *block_io_open(io_source *base)
{
    io_source *s = io_create(sizeof(block_io_data));
    if (!s) {
        return 0;
    }
    s->read_bytes = read_bytes;
    s->has_error = has_error;
    io_source *final = init_block_source(s, base);
    if (!final) {
        io_close(s);
        return 0;
    }
    return final;
}

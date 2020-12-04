#include "gogio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

io_source *io_create(size_t data_size)
{
    io_source *s = (io_source *) malloc(sizeof(io_source));
    if (!s) {
        return 0;
    }
    memset(s, 0, sizeof(io_source));
    s->data = malloc(data_size);
    if (!s->data) {
        free(s);
        return 0;
    }
    memset(s->data, 0, data_size);
    return s;
}

void io_close(io_source *s)
{
    if (s) {
        if (s->close) {
            s->close(s->data);
        }
        free(s->data);
        free(s);
    }
}

int io_error(io_source *s)
{
    return s->has_error(s->data);
}

uint8_t io_read_u8(io_source *s)
{
    uint8_t buf[1];
    if (1 == io_read_raw(s, buf, 1)) {
        return buf[0];
    } else {
        return 0;
    }
}

uint16_t io_read_u16(io_source *s)
{
    uint8_t buf[2];
    if (2 == io_read_raw(s, buf, 2)) {
        uint8_t b0 = buf[0];
        uint8_t b1 = buf[1];
        return (uint16_t) (b0 | (b1 << 8));
    } else {
        return 0;
    }
}

uint32_t io_read_u32(io_source *s)
{
    uint8_t buf[4];
    if (4 == io_read_raw(s, buf, 4)) {
        uint8_t b0 = buf[0];
        uint8_t b1 = buf[1];
        uint8_t b2 = buf[2];
        uint8_t b3 = buf[3];
        return (uint32_t) (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    } else {
        return 0;
    }
}

uint64_t io_read_u64(io_source *s)
{
    uint32_t low = io_read_u32(s);
    uint32_t high = io_read_u32(s);
    return ((uint64_t)high << 32) | low;
}

int io_read_raw(io_source *s, void *value, int max_size)
{
    return s->read_bytes(s->data, value, max_size);
}

int io_set(io_source *s, size_t offset)
{
    if (s->seek_to) {
        return s->seek_to(s->data, offset, SEEK_SET);
    } else {
        return 0;
    }
}

int io_skip(io_source *s, int size)
{
    if (s->seek_to) {
        return s->seek_to(s->data, size, SEEK_CUR);
    } else if (size > 0) {
        uint8_t tmp[16];
        int skipped = 0;
        while (size >= 16) {
            skipped += s->read_bytes(s->data, tmp, 16);
            size -= 16;
        }
        if (size) {
            skipped += s->read_bytes(s->data, tmp, size);
        }
        return skipped > 0;
    } else {
        return 0;
    }
}

long io_get_pos(io_source *s)
{
    if (s->get_pos) {
        return s->get_pos(s->data);
    } else {
        return -1;
    }
}

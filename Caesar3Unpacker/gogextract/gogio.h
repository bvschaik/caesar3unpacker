#ifndef GOG_IO_H
#define GOG_IO_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    void *data;
    int (*read_bytes)(void *source_data, void *buffer, int max_size);
    int (*seek_to)(void *source_data, long offset, int whence);
    int (*has_error)(void *source_data);
    long (*get_pos)(void *source_data);
    void (*close)(void *source_data);
} io_source;

io_source *io_create(size_t data_size);

void io_close(io_source *s);

int io_error(io_source *s);

uint8_t io_read_u8(io_source *s);

uint16_t io_read_u16(io_source *s);

uint32_t io_read_u32(io_source *s);

uint64_t io_read_u64(io_source *s);

int io_read_raw(io_source *s, void *value, int max_size);

int io_skip(io_source *s, int size);

int io_set(io_source *s, size_t offset);

long io_get_pos(io_source *s);

#endif

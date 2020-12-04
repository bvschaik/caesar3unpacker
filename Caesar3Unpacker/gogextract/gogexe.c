#include "gogexe.h"

#include "crc32.h"
#include "gogio.h"

#include <stdlib.h>
#include <string.h>

static const uint32_t RESOURCE_INSTALLER = 11111;
static const uint32_t TYPE_DATA = 10;
static const uint32_t LANGUAGE_DEFAULT = 0xffffffff;

static const uint8_t VERSION_MAGIC[12] = { 'r', 'D', 'l', 'P', 't', 'S', 0xcd, 0xe6, 0xd7, '{', 0x0b, '*' };

struct exe_header {
    int num_sections;
    uint32_t section_table_offset;
    uint32_t resource_table_address;
};

struct exe_section {
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t raw_address;
};

static int is_windows_exe(io_source *src)
{
    io_set(src, 0);
    uint16_t dos_magic = io_read_u16(src);
    io_set(src, 0x3c);
    uint16_t new_offset = io_read_u16(src);
    io_set(src, new_offset);
    uint16_t pe_magic = io_read_u32(src);

    return dos_magic == 0x5a4d && pe_magic == 0x4550 && !io_error(src);
}

static int read_header(io_source *src, struct exe_header *header)
{
    io_skip(src, 2);
    header->num_sections = io_read_u16(src);
    io_skip(src, 12); // creation time + symbol table offset + nbsymbols
    uint16_t optional_hdr_size = io_read_u16(src);
    io_skip(src, 2); // characteristics

    header->section_table_offset = (uint32_t)(io_get_pos(src)) + optional_hdr_size;

    // skip optional header
    uint16_t optional_hdr_magic = io_read_u16(src);
    if (optional_hdr_magic == 0x20b) { // PE32+
        io_skip(src, 106);
    } else {
        io_skip(src, 90);
    }

    uint32_t num_directories = io_read_u32(src);
    if (io_error(src) || num_directories < 3) {
        return 0;
    }
    io_skip(src, (4 + 4) * 2); // 2x (address + size)

	// Virtual memory address and size of the start of resource directory
    header->resource_table_address = io_read_u32(src);
    uint32_t resource_size = io_read_u32(src);
    if (io_error(src) || !header->resource_table_address || !resource_size) {
        return 0;
    }
    return 1;
}

static int read_sections(io_source *src, const struct exe_header *header, struct exe_section **sections_out)
{
    io_set(src, header->section_table_offset);

    struct exe_section *sections = (struct exe_section *) malloc(header->num_sections * sizeof(struct exe_section));
    if (!sections) {
        return 0;
    }
    for (int i = 0; i < header->num_sections; i++) {
        io_skip(src, 8); // name
        sections[i].virtual_size = io_read_u32(src);
        sections[i].virtual_address = io_read_u32(src);
        io_skip(src, 4); // raw size
        sections[i].raw_address = io_read_u32(src);
        io_skip(src, 16);
    }
    if (io_error(src)) {
        free(sections);
        return 0;
    }
    *sections_out = sections;
    return 1;
}

static uint32_t get_file_offset(struct exe_section *sections, int num_sections, uint32_t address)
{
    for (int i = 0; i < num_sections; i++) {
        struct exe_section *s = &sections[i];
        if (address >= s->virtual_address && address < s->virtual_address + s->virtual_size) {
            return address + s->raw_address - s->virtual_address;
        }
    }
	return 0;
}

static int get_resource_table(uint32_t *entry, uint32_t offset) {
	
	int is_table = ((*entry & (1 << 31)) != 0);
    *entry = *entry & ~(1 << 31);
    *entry += offset;
	return is_table;
}

uint32_t find_resource_entry(io_source *src, uint32_t id)
{
    // skip: characteristics + timestamp + major version + minor version
    io_skip(src, 12);
    if (io_error(src)) {
        return 0;
    }

    // Number of named resource entries.
    uint16_t num_names = io_read_u16(src);

    // Number of id resource entries.
    uint16_t num_ids = io_read_u16(src);

    if (id == LANGUAGE_DEFAULT) {
        io_skip(src, 4);
        uint32_t offset = io_read_u32(src);
        return io_error(src) ? 0 : offset;
    }

    // Ignore named resource entries.
    uint32_t entry_size = 4 + 4; // id / string address + offset
    io_skip(src, num_names * 8);
    if (io_error(src)) {
        return 0;
    }

    for (int i = 0; i < num_ids; i++) {
        uint32_t entry_id = io_read_u32(src);
        uint32_t entry_offset = io_read_u32(src);
        if (io_error(src)) {
            return 0;
        }
        if (entry_id == id) {
            return entry_offset;
        }
    }
    return 0;
}

static uint32_t find_resource_offset(io_source *src, struct exe_header *header, struct exe_section *sections)
{
    uint32_t resource_offset = get_file_offset(sections, header->num_sections, header->resource_table_address);
    if (!resource_offset) {
        return 0;
    }

    io_set(src, resource_offset);
    uint32_t type_offset = find_resource_entry(src, TYPE_DATA);
    if (!get_resource_table(&type_offset, resource_offset)) {
        return 0;
    }

    io_set(src, type_offset);
    uint32_t name_offset = find_resource_entry(src, RESOURCE_INSTALLER);
    if (!get_resource_table(&name_offset, resource_offset)) {
        return 0;
    }
	
    io_set(src, name_offset);
    uint32_t leaf_offset = find_resource_entry(src, LANGUAGE_DEFAULT);
    if (!leaf_offset || get_resource_table(&leaf_offset, resource_offset)) {
        return 0;
    }

    // Virtual memory address and size of the resource data.
    io_set(src, leaf_offset);
    uint32_t data_address = io_read_u32(src);
    uint32_t data_size = io_read_u32(src);
    if (io_error(src)) {
        return 0;
    }

    uint32_t data_offset = get_file_offset(sections, header->num_sections, data_address);
    if (!data_offset) {
        return 0;
    }

    return data_offset;
}

static int load_offsets(io_source *src, gog_exe_offsets *offsets)
{
    io_source *crc32 = crc32_io_attach(src);

	uint8_t magic[12];
    if (12 != io_read_raw(crc32, magic, 12)) {
        return 0;
    }

    if (0 != memcmp(magic, VERSION_MAGIC, 12)) {
        return 0;
    }

    uint8_t buf[20];
    io_read_raw(crc32, buf, 20);
    if (io_error(src) || buf[0] != 1 || buf[1] + buf[2] + buf[3] != 0) {
        // revision should be 1
        return 0;
    }

    uint32_t header_offset = io_read_u32(crc32);
    uint32_t data_offset = io_read_u32(crc32);

    uint32_t actual_checksum = crc32_io_detach(crc32);
    uint32_t expected_checksum = io_read_u32(src);
    if (io_error(src) || actual_checksum != expected_checksum) {
        return 0;
    }

    offsets->header_offset = header_offset;
    offsets->data_offset = data_offset;
    return 1;
}

int gog_exe_get_offsets(io_source *src, gog_exe_offsets *offsets)
{
    if (!is_windows_exe(src)) {
        return 0;
    }
    struct exe_header header;
    if (!read_header(src, &header)) {
        return 0;
    }
    struct exe_section *sections;
    if (!read_sections(src, &header, &sections)) {
        return 0;
    }

    uint32_t installer_offset = find_resource_offset(src, &header, sections);
    free(sections);
    if (!installer_offset) {
        return 0;
    }

    io_set(src, installer_offset);
    return load_offsets(src, offsets);
}

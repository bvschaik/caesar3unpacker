#include "gogheader.h"

#include "gogerror.h"
#include "gogioblock.h"

#include <stdlib.h>
#include <string.h>

static int version_load(io_source *src, gog_header_info *info)
{
    char version_string[65];
    io_read_raw(src, version_string, 64);
    version_string[64] = 0;

    if (strncmp("Inno Setup Setup Data (", version_string, 23) != 0) {
        return 0;
    }
    char *str = &version_string[23];
    int version[3];
    for (int i = 0; i < 3; i++) {
        char delimiter = i < 2 ? '.' : ')';
        char *end = strchr(str, delimiter);
        if (!end) {
            return 0;
        }
        *end = 0;
        version[i] = (int)strtol(str, NULL, 10);
        *end = delimiter;
        str = end + 1;
    }

    info->version.major = version[0];
    info->version.minor = version[1];
    info->version.patch = version[2];
    info->version.unicode = (strstr(version_string, "(u)") || strstr(version_string, "(U)")) ? 1 : 0;
    return 1;
}

static int version_is(gog_header_info *info, int major, int minor, int patch)
{
    return info->version.major == major && info->version.minor == minor && info->version.patch == patch;
}

static int is_supported_version(gog_header_info *info)
{
    if ((version_is(info, 5, 5, 0) || version_is(info, 5, 6, 2)) && info->version.unicode) {
        return 1;
    }
    return 0;
}

static void skip_string(io_source *src)
{
    uint32_t length = io_read_u32(src);
    io_skip(src, length);
}

static char *read_string(io_source *src)
{
    uint32_t length = io_read_u32(src);
    if (!length) {
        return NULL;
    }
    char *buffer = (char *)malloc(length);
    io_read_raw(src, buffer, length);
    for (int i = 0; i < length / 2; i++) {
        if (buffer[2*i + 1] == 0) {
            buffer[i] = buffer[2*i];
        } else {
            buffer[i] = '_';
        }
    }
    buffer[length/2] = 0;
    return buffer;
}

static void process_filename(char *buffer)
{
    char *read = buffer;
    char *write = buffer;
    int app_prefix_len = 5;
    int support_prefix_len = 14;
    if (strncmp(buffer, "{app}", app_prefix_len) == 0) {
        read += app_prefix_len;
    } else if (strncmp(buffer, "__support/app\\", support_prefix_len) == 0) {
        read += support_prefix_len;
    }
    int prev_was_slash = 1;
    while (*read) {
        int is_slash = *read == '\\' || *read == '/';
        if (!is_slash || !prev_was_slash) {
            *write = is_slash ? '/' : *read;
            write++;
        }
        read++;
        prev_was_slash = is_slash;
    }
    *write = 0;
}

static char *read_filename(io_source *src)
{
    char *buffer = read_string(src);
    if (!buffer) {
        return NULL;
    }
    process_filename(buffer);
    return buffer;
}

static void skip_windows_version(io_source *src)
{
    io_skip(src, 20);
}

static void skip_language(io_source *src)
{
    for (int i = 0; i < 10; i++) { // name, language name, fonts and license
        skip_string(src);
    }
	io_skip(src, 21); // language id, font sizes, rtl flag
}

static void skip_message(io_source *src)
{
    skip_string(src); // name
    skip_string(src); // value
    io_skip(src, 4); // language id
}

static void skip_type(io_source *src)
{
    for (int i = 0; i < 4; i++) {
        skip_string(src);
    }
    skip_windows_version(src);
    io_skip(src, 10); // flags and size
}

static void skip_component(io_source *src)
{
    for (int i = 0; i < 5; i++) {
        skip_string(src);
    }
    io_skip(src, 13);
    skip_windows_version(src);
    io_skip(src, 9); // flags and size
}

static void skip_task(io_source *src)
{
    for (int i = 0; i < 6; i++) {
        skip_string(src);
    }
    io_skip(src, 5);
    skip_windows_version(src);
    io_skip(src, 1); // flags
}

static void skip_condition_data(io_source *src)
{
    for (int i = 0; i < 6; i++) {
        skip_string(src);
    }
}

static void skip_directory(io_source *src)
{
    skip_string(src);
    skip_condition_data(src);
    io_skip(src, 4); // attributes
    skip_windows_version(src);
    io_skip(src, 3); // permission and flags
}

static void read_file_entry(io_source *src, char **filename, char **before_install, uint32_t *location, int *is_temporary)
{
    skip_string(src);
    if (filename) {
        *filename = read_filename(src);
    } else {
        skip_string(src);
    }
    skip_string(src); // font name
    skip_string(src); // assembly name

    for (int i = 0; i < 5; i++) {
        skip_string(src);
    }
    if (before_install) {
        *before_install = read_string(src);
    } else {
        skip_string(src);
    }
    skip_windows_version(src);

    uint32_t loc = io_read_u32(src);
    if (location) {
        *location = loc;
    }
    io_skip(src, 14); // attributes, size, permission
    int tmp = io_read_u8(src) & 0x8;
    if (is_temporary) {
        *is_temporary = tmp;
    }
    io_skip(src, 4); // flags
}

static int parse_before_install(const char *before_install, char **filename, int *num_locations)
{
    // We don't care about files with a before_install_dependency() script
    if (strncmp("before_install(", before_install, 15) != 0) {
        return 0;
    }
    char *open_checksum = strchr(before_install, '\'');
    if (!open_checksum) {
        return 0;
    }
    char *close_checksum = strchr(open_checksum + 1, '\'');
    if (!close_checksum) {
        return 0;
    }
    char *open_filename = strchr(close_checksum + 1, '\'');
    if (!open_filename) {
        return 0;
    }
    char *close_filename = strchr(open_filename + 1, '\'');
    if (!close_filename) {
        return 0;
    }
    size_t length = close_filename - (open_filename + 1);
    char *buffer = (char *)malloc(length + 1);
    if (!buffer) {
        return 0;
    }
    strncpy(buffer, open_filename + 1, length);
    buffer[length] = 0;
    process_filename(buffer);
    *filename = buffer;

    char *chunks = close_filename;
    while (*chunks && (*chunks < '0' || *chunks > '9')) {
        chunks++;
    }
    int locs = 0;
    if (*chunks) {
        locs = (int) strtol(chunks, NULL, 10);
    }
    *num_locations = locs ? locs : 1;
    return 1;
}

static int read_file(io_source *src, gog_header_file_entry *file, int is_indirect_filename)
{
    char *filename;
    char *before_install;
    uint32_t location;
    int is_temporary;
    read_file_entry(src, &filename, &before_install, &location, &is_temporary);

    if (!is_indirect_filename || !before_install) {
        // simple case
        free(before_install);
        file->filename = filename;
        file->num_locations = 1;
        file->locations[0] = location;
        file->is_compressed = 0;
        file->is_temporary = is_temporary || (filename && filename[0] == '{');
        return 1;
    }

    free(filename);

    // Version 5.6.2 uses indirect filenames: the actual filename is in the "before_install"
    // script. Furthermore, larger files may be split in multiple parts, and each file part
    // is zlib compressed data.
    int num_locations;
    if (!parse_before_install(before_install, &filename, &num_locations)) {
        free(before_install);
        return 1;
    }
    free(before_install);
    file->filename = filename;
    file->num_locations = num_locations;
    file->locations[0] = location;
    file->is_compressed = 1;
    file->is_temporary = is_temporary || filename[0] == '{';
    for (int i = 1; i < num_locations; i++) {
        read_file_entry(src, NULL, NULL, &file->locations[i], NULL);
    }
    return num_locations;
}

static int read_first_block(io_source *src, gog_header_info *info)
{
    int total_strings = version_is(info, 5, 5, 0) ? 31 : 34;
    for (int i = 0; i < total_strings; i++) {
        skip_string(src);
    }

    uint32_t language_count = io_read_u32(src);
    uint32_t message_count = io_read_u32(src);
    uint32_t permission_count = io_read_u32(src);
    uint32_t type_count = io_read_u32(src);
    uint32_t component_count = io_read_u32(src);
    uint32_t task_count = io_read_u32(src);
    uint32_t directory_count = io_read_u32(src);
    info->num_files = io_read_u32(src);
    info->num_data_entries = io_read_u32(src);
    uint32_t icon_count = io_read_u32(src);
    uint32_t ini_entry_count = io_read_u32(src);
    uint32_t registry_entry_count = io_read_u32(src);
    uint32_t delete_entry_count = io_read_u32(src);
    uint32_t uninstall_delete_entry_count = io_read_u32(src);
    uint32_t run_entry_count = io_read_u32(src);
    uint32_t uninstall_run_entry_count = io_read_u32(src);

    skip_windows_version(src);

    // Colors
    if (version_is(info, 5, 5, 0)) {
        io_skip(src, 12);
    } else {
        io_skip(src, 9);
    }

    // Password stuff
    io_skip(src, 28);

    // Extra disk and other enum flags
    io_skip(src, 17);
    uint8_t compression = io_read_u8(src);
    if (compression != 4) {
        gog_set_error("Unknown compression method: %d", compression);
        return 0;
    }

    // Architecture, disabled dir, uninstall size
    io_skip(src, 12);

    // Options flag bitvector
    io_skip(src, 6);

    for (int i = 0; i < language_count; i++) {
        skip_language(src);
    }
    for (int i = 0; i < message_count; i++) {
        skip_message(src);
    }
    for (int i = 0; i < permission_count; i++) {
        skip_string(src); // single string
    }
    for (int i = 0; i < type_count; i++) {
        skip_type(src);
    }
    for (int i = 0; i < component_count; i++) {
        skip_component(src);
    }
    for (int i = 0; i < task_count; i++) {
        skip_task(src);
    }
    for (int i = 0; i < directory_count; i++) {
        skip_directory(src);
    }
    info->files = calloc(info->num_files, sizeof(gog_header_file_entry));
    if (!info->files) {
        return 0;
    }
    int is_indirect = version_is(info, 5, 6, 2);
    for (int i = 0; i < info->num_files;) {
        i += read_file(src, &info->files[i], is_indirect);
    }
    // Skip to EOF
    while (io_skip(src, 4096));
    return 1;
}

static void read_data_entry(io_source *src, gog_header_data_entry *entry)
{
    entry->chunk_first_slice = io_read_u32(src);
    entry->chunk_last_slice = io_read_u32(src);
    entry->chunk_offset = io_read_u32(src);

	entry->file_offset = io_read_u64(src);
    entry->file_size = io_read_u64(src);
    entry->chunk_size = io_read_u64(src);

    io_read_raw(src, entry->sha1_checksum, 20);

    io_skip(src, 16); // file time, file version

    uint16_t flags = io_read_u16(src);
    entry->compressed = (flags & 0x80) == 0 ? 0 : 1;
    entry->exe_optimized = (flags & 0x10) == 0 ? 0 : 1;
}

static int read_second_block(io_source *src, gog_header_info *info)
{
    info->data_entries = calloc(info->num_data_entries, sizeof(gog_header_data_entry));
    if (!info->data_entries) {
        return 0;
    }
    for (int i = 0; i < info->num_data_entries; i++) {
        read_data_entry(src, &info->data_entries[i]);
    }
    return 1;
}

static int read_block(io_source *src, gog_header_info *info, int (*reader)(io_source*, gog_header_info*))
{
    io_source *block = block_io_open(src);
    if (!block) {
        return 0;
    }
    int result = reader(block, info);
    io_close(block);
    return result;
}

int gog_header_load(io_source *src, gog_header_info *info)
{
    if (!version_load(src, info)) {
        gog_set_error("Unable to read InnoSetup version");
        return 0;
    }
    if (!is_supported_version(info)) {
        gog_set_error("Unsupported InnoSetup version %d.%d.%d",
            info->version.major, info->version.minor, info->version.patch);
        return 0;
    }
    return read_block(src, info, read_first_block) && read_block(src, info, read_second_block);
}

void gog_header_free(gog_header_info *info)
{
    if (info->files) {
        for (int i = 0; i < info->num_files; i++) {
            if (info->files[i].filename) {
                free(info->files[i].filename);
            }
        }
        free(info->files);
        info->files = 0;
    }
    info->num_files = 0;

    free(info->data_entries);
    info->data_entries = 0;
    info->num_data_entries = 0;
}

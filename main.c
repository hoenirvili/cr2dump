#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fort.h>
#include <assert.h>

#include "tag_type.h"

// Kilobytes in bytes
static const size_t KB = 1024;

// Megabytes in bytes
static const size_t MB = KB * 1024;

// MB transform bytes to megabytes
static float mb(size_t bytes) { return bytes / MB; }

static const int quit_key = 'q';

static bool is_quit(int key) { return (toupper(key) == toupper(quit_key)); }

// fsize returns the file size in bytes
static int fsize(FILE *fp)
{
    struct stat buf;
    int err = fstat(fileno(fp), &buf);
    if (err < 0)
        return err;
    return buf.st_size;
}

#define BYTE_ORDER_CHARS 2

#define HEADER_LITTLE_ENDIAN "II"

#define HEADER_BIG_ENDIAN "MM"

#define MAGIC_WORD_CHARS 2

// A Canon CRW file starts with the following byte sequence
// contains the byte ordering, the version and the offset to the RAW picture
struct file_header {
    char byte_order[BYTE_ORDER_CHARS];
    short tiff_magic_word;
    unsigned int tiff_offset;
    char cr2_magic_word[MAGIC_WORD_CHARS]; // or 0x4352
    char cr2_major_version;
    char cr2_minor_version;
    unsigned int raw_ifd_offset;
}__attribute__((packed));

// this part contains the Exif section, which contains the Makernotes section.
// Information about picture#0.
#define SLOTS_PER_ENTRY 12

struct ifd {
    uint16_t number_of_entries;
    struct entry *entries;
    unsigned int next_ifd_offset;
};


// A TIFF tag, is a logical entity which consist in:
// an record (Directory Entry) inside an IFD, and some data.
// These two parts are generally separated.
// All directory entries are a sequence
// inside the same IFD, the data can be anywhere in the file.
struct entry {
    uint16_t tag_id;
    uint16_t tag_type;
    uint32_t number_of_value;
    uint32_t value; // value, or pointer to the data
}__attribute__((packed));

static void dump_file_header(struct file_header header)
{
    puts("HEADER");
    ft_table_t *table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table,
            "byte_order", "tiff_magic_word", "tiff_offset", "crw_magic_word",
            "cr2_major_version", "cr2_minor_version", "raw_ifd_offset");
    ft_printf_ln(table, "%2.2s|%#06hx|%#08x|%2.2s|%d|%d|%#08x",
            header.byte_order,
            header.tiff_magic_word,
            header.tiff_offset,
            header.cr2_magic_word,
            header.cr2_major_version,
            header.cr2_minor_version,
            header.raw_ifd_offset);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
}

static void parse_file_header(FILE *fp, struct file_header *header)
{
    fread(header, 1, sizeof(*header), fp);
    rewind(fp);
}

static void parse_ifd(FILE *fp, unsigned int ifd_offset, struct ifd *ifd)
{
    fseek(fp, ifd_offset, SEEK_SET);
    fread(&ifd->number_of_entries, 1, sizeof(ifd->number_of_entries), fp);
    ifd->entries = malloc(sizeof(struct entry) * ifd->number_of_entries);
    if (ifd->entries == NULL) {
        puts("malloc() number_of_entries failed");
        exit(1);
    }
    for (uint16_t i = 0; i<ifd->number_of_entries; i++)
        fread(&ifd->entries[i], 1, sizeof(ifd->entries[i]), fp);

    fread(&ifd->next_ifd_offset, 1, sizeof(ifd->next_ifd_offset), fp);
    rewind(fp);
}

static void free_ifd_entries(struct ifd ifd) { free(ifd.entries); }

static void dump_ifd(struct ifd ifd, int count)
{
    printf("IFD%d\n", count);
    ft_table_t *table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "number_of_entries", "entries", "next_ifd_offset");
    ft_printf_ln(table, "%d|in mem %p|%#08x", ifd.number_of_entries, ifd.entries, ifd.next_ifd_offset);
    printf("%s", ft_to_string(table));
    ft_destroy_table(table);

    printf("IFD%d addr entries %p details\n", count, ifd.entries);
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "tag_id", "tag_type", "number_of_values", "value_or_start_addr_to_data");
    for (uint16_t i = 0; i < ifd.number_of_entries; i++)
        ft_printf_ln(table, "%d|%s|%d|%#08x",
                ifd.entries[i].tag_id,
                tag_type_to_field_str(get_tag_type(ifd.entries[i].tag_type)),
                ifd.entries[i].number_of_value,
                ifd.entries[i].value);
    printf("%s", ft_to_string(table)); ft_destroy_table(table);
}

// The .CR2 file is based on the TIFF file format
// This TIFF file has 4 Image File Directories (IFDs).
#define MAX_NUMBER_OF_IFDS 4 // I'm not sure to be honest

static void dump_ifds(struct ifd *ifds, size_t nfds)
{
    for (int i = 0; i < nfds; i++)
        dump_ifd(ifds[i], i);
}

static void parse_all_ifds(
        FILE *fp,
        unsigned int tiff_offset,
        struct ifd *ifds,
        size_t nfds)
{
    for (size_t i = 0; i < nfds; i++) {
        parse_ifd(fp, tiff_offset, &ifds[i]);
        tiff_offset = ifds[i].next_ifd_offset;
    }
}

static_assert((sizeof(struct entry) == SLOTS_PER_ENTRY));

int main(int argc, char **argv)
{
    if (argc <= 1) {
        puts("Pass file path to CR2 file\n");
        return 1;
    }

    const char *filename = argv[1];

    FILE *cr = fopen(filename, "r");
    if (cr == NULL) {
        printf("Cannot open file %s\n", argv[1]);
        return 1;
    }

    puts("Extra information");
    printf("File: %s\n", filename);
    size_t bytes = fsize(cr);
    float mbytes = mb(bytes);
    printf("File size: %.4f MB\n", mbytes);

    struct file_header header;
    parse_file_header(cr, &header);
    if (!strncmp(header.byte_order, HEADER_LITTLE_ENDIAN, BYTE_ORDER_CHARS)) {
        puts("File: is little endian format\n\n");
    } else if (!strncmp(header.byte_order, HEADER_BIG_ENDIAN, BYTE_ORDER_CHARS)) {
        puts("File is big endian format\n\n");
    }

    dump_file_header(header);

    struct ifd ifds[MAX_NUMBER_OF_IFDS] = { 0 };
    parse_all_ifds(cr, header.tiff_offset, ifds, MAX_NUMBER_OF_IFDS);
    dump_ifds(ifds, MAX_NUMBER_OF_IFDS);

    for (size_t i = 0; i < MAX_NUMBER_OF_IFDS; i++ )
        free_ifd_entries(ifds[i]);

    fclose(cr);
}

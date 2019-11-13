#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "tag_type.h"

// Kilobytes in bytes
static const size_t KB = 1024;

// Megabytes in bytes
static const size_t MB = KB * 1024;

// MB transform bytes to megabytes
static float mb(size_t bytes) { return bytes / MB; }

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
struct cr2_header{
    char canon_raw_marker[MAGIC_WORD_CHARS];
    uint16_t version;
    uint32_t offset_to_raw_ifd;
}__attribute__((packed));

static void dump_cr2_header(struct cr2_header hdr)
{
    puts("\nCR2 Header\n");
    printf("canon_raw_marker: %2.2s\n", hdr.canon_raw_marker);
    printf("version: %d\n", hdr.version);
    printf("offset_to_raw_ifd: %#08x\n", hdr.offset_to_raw_ifd);
}

static_assert(sizeof(struct cr2_header) == 8,
        "cr2 header should be 8 bytes");

struct tiff_header {
    char endianness[BYTE_ORDER_CHARS];
    uint16_t magic_number;
    uint32_t offset_to_ifd;
}__attribute__((packed));

static_assert(sizeof(struct tiff_header) == 8,
        "tiff header should be 8 bytes");

static void dump_tiff_header(struct tiff_header hdr)
{
    puts("\nTIFF header\n");
    printf("endianness: %2.2s\n", hdr.endianness);
    printf("magic_number: %#06hx\n", hdr.magic_number);
    printf("offset_to_ifd: %#08x\n", hdr.offset_to_ifd);
}

// this part contains the Exif section, which contains the Makernotes section.
// Information about picture#0.
#define SLOTS_PER_ENTRY 12

struct ifd {
    uint16_t number_of_entries;
    struct entry *entries;
    unsigned int next_ifd_offset;
};

struct maker_note_ifd {
    struct ifd ifd;
    struct tiff_header hdr;
};

// A TIFF tag, is a logical entity which consist in:
// an record (Directory Entry) inside an IFD, and some data.
// These two parts are generally separated.
// All directory entries are a sequence
// inside the same IFD, the data can be anywhere in the file.
struct entry {
    uint16_t tag_id;
    uint16_t tag_type;
    uint32_t number_of_value; // maybe if this is 1 then it's a value and
    // if this is > 1 it's a ptr?
    uint32_t value; // value, or pointer to the data
}__attribute__((packed));

static struct entry ifd_get_entry(struct ifd ifd, size_t i)
{
    assert(i < ifd.number_of_entries);
    return ifd.entries[i];
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

static void dump_ifd(FILE *fp, struct ifd ifd, int count)
{
    printf("IFD%d number_of_entries: %d, in mem: %p, next_ifd_offset: %#08x\n\n",
            count,
            ifd.number_of_entries,
            ifd.entries,
            ifd.next_ifd_offset);
    for (uint16_t i = 0; i < ifd.number_of_entries; i++) {
        printf("entry number: %d ", i);
        struct entry entry = ifd_get_entry(ifd, i);
        enum tag_type tag = get_tag_type(entry.tag_type);
        const char *tag_str = tag_type_to_field_str(tag);
        const char *payload = "";
        if (entry.value)
            payload = tag_type_conv(
                fp, tag, entry.value, entry.number_of_value);
        printf("tag_id: %d|%#04x, ", entry.tag_id, entry.tag_id);
        printf("tag_type: %s, ", tag_str);
        printf("number_of_values: %d, ", entry.number_of_value);
        printf("value_or_start_addr_to_data: %#08x, ", entry.value);
        printf("unmarshal value: \"%s\"\n", payload);
    }
}

#define TAG_ID_MAKER_NOTE 0x927c

// exif which consists of the same memory layout as an struct ifd
typedef struct ifd exif;

// The .CR2 file is based on the TIFF file format
// This TIFF file has 4 Image File Directories (IFDs).
#define MAX_NUMBER_OF_IFDS 4

static void dump_ifds(FILE *fp, struct ifd *ifds, size_t nfds)
{
    for (int i = 0; i < nfds; i++) {
        puts("\n");
        dump_ifd(fp, ifds[i], i);
    }
}

#define TAG_ID_EXIF 0x8769

static uint16_t ifd_exif_tag_value(const struct ifd *ifd)
{
    for (size_t i = 0; i < ifd->number_of_entries; i++)
        if (ifd->entries[i].tag_id == TAG_ID_EXIF)
            return ifd->entries[i].value;
    return 0;
}

static void parse_exif_sub_ifd(const struct ifd *ifd, exif *exif, FILE *fp)
{
    uint16_t value = ifd_exif_tag_value(ifd);
    if (!value) {
        printf("ifd_exif_tag_value() failed");
        exit(1);
    }
    fseek(fp, value, SEEK_SET);
    fread(exif, sizeof(*exif), 1, fp);
    rewind(fp);
}

static uint16_t exif_maker_note_value(const exif *exif)
{
    for (size_t i = 0; i < exif->number_of_entries; i++)
        if (exif->entries[i].tag_id == TAG_ID_MAKER_NOTE)
            return exif->entries[i].value;
    return 0;
}

static void parse_maker_note_ifd(
        const exif *exif,
        struct maker_note_ifd *mnifd,
        FILE *fp)
{
    uint16_t value = exif_maker_note_value(exif);
    if (!value) {
        printf("ifd_exif_tag_value() failed");
        exit(1);
    }
    parse_ifd(fp, value, &mnifd->ifd);
    fseek(fp, value + mnifd->ifd.number_of_entries * sizeof(mnifd->ifd.entries), SEEK_SET);
    rewind(fp);
}

static void parse_all_ifds(
        FILE *fp,
        unsigned int tiff_offset,
        struct ifd *ifds,
        size_t nfds,
        struct ifd *exif,
        struct maker_note_ifd *mnifd)
{
    for (size_t i = 0; i < nfds; i++) {
        parse_ifd(fp, tiff_offset, &ifds[i]);
        tiff_offset = ifds[i].next_ifd_offset;
        if (i == 0) {
            parse_exif_sub_ifd(&ifds[i], exif, fp);
            //TODO: fix this
            //parse_maker_note_ifd(exif, mnifd, fp);
        }
    }
}

static_assert(sizeof(struct entry) == SLOTS_PER_ENTRY,
        "this width of an entry should always be fixed to SLOTS_PER_ENTRY");

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

    struct tiff_header tiffhdr;
    fread(&tiffhdr, sizeof(tiffhdr), 1, cr);
    if (!strncmp(tiffhdr.endianness, HEADER_LITTLE_ENDIAN, BYTE_ORDER_CHARS)) {
        puts("\nFile: is little endian format\n\n");
    } else if (!strncmp(tiffhdr.endianness, HEADER_BIG_ENDIAN, BYTE_ORDER_CHARS)) {
        puts("\nFile is big endian format\n\n");
    }
    dump_tiff_header(tiffhdr);
    struct cr2_header cr2hdr;
    fread(&cr2hdr, sizeof(cr2hdr), 1, cr);
    dump_cr2_header(cr2hdr);

    struct ifd ifds[MAX_NUMBER_OF_IFDS];
    exif exif_sub_ifd;
    struct maker_note_ifd mnifd;
    parse_all_ifds(cr, tiffhdr.offset_to_ifd, ifds, MAX_NUMBER_OF_IFDS, &exif_sub_ifd, &mnifd);
    dump_ifds(cr, ifds, MAX_NUMBER_OF_IFDS);
    for (size_t i = 0; i < MAX_NUMBER_OF_IFDS; i++ )
        free_ifd_entries(ifds[i]);

    fclose(cr);
}

#include <stdlib.h>
#include "ifd.h"
#include "error.h"
#include "tag_type.h"

void ifd_parse(struct ifd *ifd, size_t offset, FILE *cr)
{
    fseek(cr, offset, SEEK_SET);
    fread(&ifd->number_of_entries, 1, sizeof(ifd->number_of_entries), cr);
    ifd->entries = malloc(sizeof(struct entry) * ifd->number_of_entries);
    malloc_check(ifd->entries, "malloc() number_of_entries, failed");
    for (uint16_t i = 0; i < ifd->number_of_entries; i++)
        fread(&ifd->entries[i], 1, sizeof(ifd->entries[i]), cr);
    fread(&ifd->next_ifd_offset, 1, sizeof(ifd->next_ifd_offset), cr);
    rewind(cr);
}

void ifds_parse(struct ifd *ifds, size_t count, size_t offset, FILE *cr)
{
    for (size_t i = 0, off = offset; i < count; i++) {
        ifd_parse(&ifds[i], off, cr);
        off = ifds[i].next_ifd_offset;
    }
}

void ifd_entries_free(struct ifd ifd)
{
    free(ifd.entries);
}

void ifds_entries_free(struct ifd *ifds, size_t count)
{
    for (size_t i = 0; i < count; i++)
        free(ifds[i].entries);
}

static inline struct entry ifd_entry(const struct ifd ifd, size_t i)
{
    return ifd.entries[i];
}

void entry_dump(struct entry entry, FILE *cr, FILE *out, int i)
{
    printf("entry number: %d ", i);
    enum tag_type tag = tag_type(entry.tag_type);
    const char *tag_str = tag_type_to_field_str(tag);
    const char *payload = "";
    if (entry.value)
        payload = tag_type_conv(
                cr, tag, entry.value, entry.number_of_value);
    fprintf(out, "tag_id: %d|%#04x, ", entry.tag_id, entry.tag_id);
    fprintf(out, "tag_type: %s, ", tag_str);
    fprintf(out, "number_of_values: %d, ", entry.number_of_value);
    fprintf(out, "value_or_start_addr_to_data: %#08x ", entry.value);
    fprintf(out, "unmarshal value: \"%s\"\n", payload);
}

static const char *dump_fmt = "number_of_entries: %d, in mem: %p, next_ifd_offset: %#08x\n\n";

void ifd_dump(struct ifd ifd, FILE *cr, FILE *out)
{
    fprintf(out, dump_fmt, ifd.number_of_entries, ifd.entries, ifd.next_ifd_offset);
    size_t n = ifd.number_of_entries;
    for (size_t i = 0; i < n; i++)
        entry_dump(ifd_entry(ifd, i), cr, out, i);
}

void ifds_dump(struct ifd *ifds, size_t count, FILE *cr, FILE *out)
{
    for (size_t i = 0; i < count; i++) {
        fprintf(out, "\n\nIFD%lu\n", i);
        ifd_dump(ifds[i], cr, out);
    }
}

#define TAG_ID_EXIF 0x8769

static uint16_t ifd_value(struct ifd ifd, uint16_t value)
{
    for (size_t i = 0; i < ifd.number_of_entries; i++)
        if (ifd.entries[i].tag_id == value)
            return ifd.entries[i].value;
    return 0;
}

uint16_t ifd_exif_value(struct ifd ifd)
{
    return ifd_value(ifd, TAG_ID_EXIF);
}

#define TAG_ID_MAKER_NOTE 0x927c

uint16_t ifd_maker_note(struct ifd exif)
{
    return ifd_value(exif, TAG_ID_MAKER_NOTE);
}

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

// this part contains the Exif section, which contains the Makernotes section.
// Information about picture#0.
#define SLOTS_PER_ENTRY 12

#define N_IFDS 4

struct ifd {
    uint16_t number_of_entries;
    struct entry *entries;
    unsigned int next_ifd_offset;
};

extern void ifd_entries_free(struct ifd ifd);

extern void ifd_parse(struct ifd *ifd, size_t offset, FILE *out);

extern void ifd_dump(struct ifd ifd, FILE *cr, FILE *out);

extern void ifds_dump(struct ifd *ifds, size_t count, FILE *cr, FILE *out);

extern void ifds_parse(struct ifd *ifd, size_t count, size_t offset, FILE *cr);

extern void ifds_entries_free(struct ifd *ifds, size_t count);

extern uint16_t ifd_exif_value(struct ifd ifd);

extern uint16_t ifd_maker_note(struct ifd ifd);

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

static_assert(sizeof(struct entry) == SLOTS_PER_ENTRY,
        "this width of an entry should always be fixed to SLOTS_PER_ENTRY");


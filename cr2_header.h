#pragma once

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define MAGIC_WORD_CHARS 2

// A Canon CRW file starts with the following byte sequence
// contains the byte ordering, the version and the offset to the RAW picture
struct cr2_header{
    char canon_raw_marker[MAGIC_WORD_CHARS];
    uint16_t version;
    uint32_t offset_to_raw_ifd;
}__attribute__((packed));

static_assert(sizeof(struct cr2_header) == 8,
        "cr2 header should be 8 bytes");

extern void cr2_header_parse(struct cr2_header *hdr, FILE *fp);
extern void cr2_header_dump(struct cr2_header hdr, FILE *fp);

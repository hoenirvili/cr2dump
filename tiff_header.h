#pragma once

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define BYTE_ORDER_CHARS 2


#define HEADER_LITTLE_ENDIAN "II"

#define HEADER_BIG_ENDIAN "MM"


struct tiff_header {
    char endianness[BYTE_ORDER_CHARS];
    uint16_t magic_number;
    uint32_t offset_to_ifd;
}__attribute__((packed));

static_assert(sizeof(struct tiff_header) == 8,
        "tiff header should be 8 bytes");

extern void tiff_header_dump(struct tiff_header hdr, FILE *fp);

extern void tiff_header_parse(struct tiff_header *hdr, FILE *fp);


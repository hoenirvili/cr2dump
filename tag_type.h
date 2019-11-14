#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum tag_type {
    no_tag,                 /* here to occupy 0 */
    tag_unsigned_char,      /* 1 = unsigned char */
    tag_string,             /* 2 = string (with an ending zero) */
    tag_unsigned_short,     /* 3 = unsigned short (2 bytes) */
    tag_unsigned_long,      /* 4 = unsigned long (4 bytes) */
    tag_unsigned_rational,  /* 5 = unsigned rational (2 unsigned long) */
    tag_signed_char,        /* 6 = signed char */
    tag_byte_sequence,      /* 7 = byte sequence */
    tag_signed_short,       /* 8 = signed short */
    tag_signed_long,        /* 9 = signed long */
    tag_signed_rational,    /* 10 = signed rational (2 signed long) */
    tag_float_4bytes,       /* 11 = float, 4 bytes, IEEE format */
    tag_float_8bytes,       /* 12 = float, 8 bytes, IEEE format */
    end_tag                 /* 13 = end of the enum */
};

#define TAG_TYPE_ENUM_COUNT end_tag

extern const char *tag_type_to_field_str(enum tag_type t);

extern const char *tag_type_conv(FILE *fp, enum tag_type t, uint32_t addr, size_t count);

extern enum tag_type tag_type(uint16_t t);

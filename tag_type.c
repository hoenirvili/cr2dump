#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "tag_type.h"


//TODO(""): use this for conversion with ratitonsl
struct signed_rational { long p, q; };
struct unsigned_rationl { unsigned long p,q; };

static const char *tag_type_str_list[] = {
    "no tag specified",
    "unsigned char",
    "string (with an ending zero",
    "unsigned short (2 bytes)",
    "unsigned long (4 bytes)",
    "unsigned rationl (2 unsinged long)",
    "signed char",
    "byte sequence",
    "signed short",
    "signed long",
    "signed rationl (2 signed long)",
    "float, 4 bytes, IEEE format",
    "floart, 8 bytes, IEEE format",
    NULL,
};

enum tag_type get_tag_type(uint16_t t)
{
    return (enum tag_type)(t);
}

// let it be 8k bytes, should be enough, (IDK to be honst)
#define MAX_BUFFER_SIZE 8192 + 1

// we could alloc memory for ever str conversion but I'm a cheap asshole
static char buffer[MAX_BUFFER_SIZE];

static const char *no_tag_specified_fn(const void *addr, size_t count) { return NULL; }

static const char *unsigned_char_fn(const void *addr, size_t count)
{
    memset(buffer, 0, sizeof(buffer));
    assert((count * sizeof(unsigned char)) < sizeof(buffer));
    memcpy(buffer, addr, count * sizeof(unsigned char));
    return buffer;
}

static const char *string_fn(const void *addr, size_t count)
{
    // count => includes the ending '\0' yoo
    // we could use strcpy because we have the '\0' but just to be extra
    // mega triple sure let's use memcpy, anyway memcpy will be optimized
    // so yoooooooo
    return unsigned_char_fn(addr, count);
}

#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])

/* static const char *unsigned_short_fn(const void *addr, size_t count) */
/* { */
/*     memset(&buffer[0], 0, ARRAY_SIZE(buffer)); */
/*     const uint16_t *value = addr; */
/*     size_t n = 0; // how much we wrote */
/*     char *pos = buffer; */
/*     ssize_t bytes_left = MAX_BUFFER_SIZE; */
/*     size_t i = 0; */
/*     const char *fmt = "%hu "; // yeah I could just pos + the position and assign ' ' */
/*     const char *end_fmt = "%hu"; */
/*     for (;i <count; i++) { */
/*         if (i + 1 == count) */
/*             fmt = end_fmt; */
/*         n = snprintf(pos, bytes_left, fmt, value[i]); */
/*         if (n < 0) { */
/*             puts("snprintf() to buffer failed"); */
/*             exit(1); */
/*         } */
/*         bytes_left -= n; // basically we try to protect how much we write */
/*         // and don't overrun the buffer */
/*         assert("the buffer should be bigger" && bytes_left > 0); */
/*         pos = &pos[n]; */
/*     }; */
/*     return buffer; */
/* } */

#define CONVERSION_FN(type, tfmt) \
static const char *##type_fn(const void *addr, size_t count)                    \
{                                                                               \
    memset(&buffer[0], 0, ARRAY_SIZE(buffer));                                  \
    const type *value = addr;                                                   \
    size_t n = 0;                                                               \
    char *pos = buffer;                                                         \
    ssize_t bytes_left = MAX_BUFFER_SIZE;                                       \
    size_t i = 0;                                                               \
    const char *fmt = #tfmt" ";                                                 \
    const char *end_fmt = #tfmt;                                                \
    for (;i <count; i++) {                                                      \
        if (i + 1 == count)                                                     \
            fmt = end_fmt;                                                      \
        n = snprintf(pos, bytes_left, fmt, value[i]);                           \
        if (n < 0) {                                                            \
            puts("snprintf() to buffer failed");                                \
            exit(1);                                                            \
        }                                                                       \
        bytes_left -= n;                                                        \
        assert("the buffer should be bigger" && bytes_left > 0);                \
        pos = &pos[n];                                                          \
    };                                                                          \
    return buffer;                                                              \
}

CONVERSION_FN(uint16_t, "%hu")


static const char *unsigned_long_fn(const void *addr, size_t count)
{
    return NULL;
}

static const char *unsigned_rational_fn(const void *addr, size_t count)
{
    // TODO(""): not implemented yet
    return NULL;
}

static const char *signed_char_fn(const void *addr, size_t count)
{
    // TODO(""): not implemented yet
    return NULL;
}

static const char *byte_sequence_fn(const void *addr, size_t count)
{
    // TODO(""): not implemented yet
    return NULL;
}

static const char *signded_short_fn(const void *addr, size_t count)
{
    // TODO(""): not implemented yet
    return NULL;
}

static const char *signed_long_fn(const void *addr, size_t count)
{
    // TODO(""): not implemented yet
    return NULL;
}

static const char *signed_rational_fn(const void *addr, size_t count)
{
    //TODO(""): not implemented yet
    return NULL;
}

static const char *float_4byte_fn(const void *addr, size_t count)
{
    //TODO(""): not implemented yet
    return NULL;
}

static const char *float_8byte_fn(const void *addr, size_t count)
{
    //TODO(""): not implemented yet
    return NULL;
}

#define END (tag_type_table){}

struct tag_type_table {
    const char *(*convert)(const void *addr, size_t count);
}tag_type_table[] = {
    {no_tag_specified_fn},
    {}
};

const char *tag_type_conv(enum tag_type t, const void* addr, size_t count)
{
    return tag_type_table[t].convert(addr, count);
}

const char *tag_type_to_field_str(enum tag_type t)
{
    // if this fails then I know that the user(me) fucked it up. :)
    assert("we added or removed an enum" && t > 0 && t < TAG_TYPE_ENUM_COUNT);
    static_assert(ARRAY_SIZE(tag_type_str_list)-1 == TAG_TYPE_ENUM_COUNT,
            "tag_type_str_listt should have the same members as the enum");
    return tag_type_str_list[t];
}



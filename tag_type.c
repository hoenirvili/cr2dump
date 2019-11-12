#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "tag_type.h"

// let it be 8k bytes, should be enough, (IDK to be honst)
#define MAX_BUFFER_SIZE 8192 + 1

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static bool is_bad_char(char a)
{
    return (isspace(a) || a == '\n');
}

static size_t next_good_char_at(char *buff, size_t from, size_t n)
{
    for (size_t i = from; i < n; i++)
        if (!is_bad_char(buff[i]))
            return i;
    return 0;
}

static void fold(char *buff, size_t from, size_t to, size_t n)
{
    assert(from < to);
    while (to < n) {
        buff[from] = buff[to];
        from++;
        to++;
    }
    assert(from < n);
    memset(&buff[from], 0, n - from);
}

static void trim_unnecessary_chars(char *buff, size_t n)
{
    size_t to = 0;
    for (size_t from = 0; from < n; from++) {
        if (is_bad_char(buff[from])) {
            to = next_good_char_at(buff, from+1, n);
            if (to == 0)
                break;
            fold(buff, from, to, n);
        }
    }
}
// we could alloc memory for ever str conversion but I'm a cheap asshole
static char buffer[MAX_BUFFER_SIZE];

static const char *no_tag_specified_fn(FILE *fp, uint32_t addr, size_t count) { return NULL; }

static_assert(sizeof(char) == 1, "char is one byte");

static const char *unsigned_char_fn(FILE *fp, uint32_t addr, size_t count)
{
    assert(MAX_BUFFER_SIZE > count);
    memset(buffer, 0, sizeof(buffer));
    fseek(fp, addr, SEEK_SET);
    fread(buffer, 1, count, fp);
    rewind(fp);
    trim_unnecessary_chars(buffer, count);
    return buffer;
}


static const char *string_fn(FILE *fp, uint32_t addr, size_t count)
{
    // count => includes the ending '\0' yoo
    // we could use strcpy because we have the '\0' but just to be extra
    // mega triple sure let's use memcpy, anyway memcpy will be optimized
    // so yoooooooo
    return unsigned_char_fn(fp, addr, count);
}

#define WHEN_TO_ADD_NEWLINE 50

struct signed_rational { long p, q; };
struct unsigned_rational { unsigned long p, q; };

#define CONVERSION_RATIONAL_FN(name, type, tfmt)                            \
static const char *name##_fn(FILE *fp, uint32_t addr, size_t count)         \
{                                                                           \
    type _buffer[count];                                                    \
    fseek(fp, addr, SEEK_SET);                                              \
    fread(_buffer, sizeof(_buffer[0]), count, fp);                          \
    const char *sfmt = #tfmt" ";                                            \
    const char *nfmt = #tfmt"\n";                                           \
    ssize_t left_over = sizeof(buffer);                                     \
    ssize_t n = 0;                                                          \
    size_t add_line = 0;                                                    \
    size_t current = 0;                                                     \
    for (size_t i = 0; i < count; i++) {                                    \
        const char *fmt = sfmt;                                             \
        n = snprintf(NULL, 0, fmt, _buffer[i].p, _buffer[i].q);             \
        if (n < 0) {                                                        \
            printf("sprintf count failed\n");                               \
            exit(1);                                                        \
        }                                                                   \
        left_over = left_over - n;                                          \
        if (left_over <= 0) {                                               \
            printf("cannot write more bytes into buffer\n");                \
            exit(1);                                                        \
        }                                                                   \
        add_line += n;                                                      \
        if (add_line >= WHEN_TO_ADD_NEWLINE) {                              \
            fmt = nfmt;                                                     \
            add_line = 0;                                                   \
        }                                                                   \
        n = sprintf(&buffer[current], fmt, _buffer[i].p, _buffer[i].q);     \
        if (n < 0) {                                                        \
            printf("sprintf() failed\n");                                   \
            exit(1);                                                        \
        }                                                                   \
        current += n;                                                       \
    }                                                                       \
    rewind(fp);                                                             \
    buffer[strlen(buffer)-1] = 0;                                           \
    return buffer;                                                          \
}

CONVERSION_RATIONAL_FN(unsigned_rational, struct unsigned_rational, %#08x/%#08x)
CONVERSION_RATIONAL_FN(signed_rational, struct signed_rational, %#08x/%#08x)

#define CONVERSION_FN(name, type, tfmt)                                         \
static const char *name##_fn(FILE *fp, uint32_t addr, size_t count)             \
{                                                                               \
    type _buffer[count];                                                        \
    if (count > 1) {                                                            \
        fseek(fp, addr, SEEK_SET);                                              \
        fread(&_buffer[0], sizeof(_buffer[0]), count, fp);                      \
    } else {                                                                    \
        memcpy(_buffer, &addr, sizeof(_buffer[0]));                             \
    }                                                                           \
    const char *sfmt = #tfmt" ";                                                \
    const char *nfmt = #tfmt"\n";                                               \
    ssize_t left_over = sizeof(buffer);                                         \
    ssize_t n = 0;                                                              \
    size_t add_line = 0;                                                        \
    size_t current = 0;                                                         \
    for (size_t i = 0; i < count; i++) {                                        \
        const char *fmt = sfmt;                                                 \
        n = snprintf(NULL, 0, fmt, _buffer[i]);                                 \
        if (n < 0) {                                                            \
            printf("sprintf count failed\n");                                   \
            exit(1);                                                            \
        }                                                                       \
        left_over = left_over - n;                                              \
        if (left_over <= 0) {                                                   \
            printf("cannot write more bytes into buffer\n");                    \
            exit(1);                                                            \
        }                                                                       \
        add_line += n;                                                          \
        if (add_line >= WHEN_TO_ADD_NEWLINE) {                                  \
            fmt = nfmt;                                                         \
            add_line = 0;                                                       \
        }                                                                       \
        n = sprintf(&buffer[current], fmt, _buffer[i]);                         \
        if (n < 0) {                                                            \
            printf("sprintf() failed\n");                                       \
            exit(1);                                                            \
        }                                                                       \
        current += n;                                                           \
    }                                                                           \
    if (count > 1)                                                              \
        rewind(fp);                                                             \
    buffer[strlen(buffer)-1] = 0;                                               \
    return buffer;                                                              \
}

CONVERSION_FN(unsigned_short, uint16_t, %hu)
CONVERSION_FN(unsigned_long, uint32_t, %d)
CONVERSION_FN(signed_char, char, %c)
CONVERSION_FN(byte_sequence, uint8_t, %x)
CONVERSION_FN(signed_short, int16_t, %d)
CONVERSION_FN(signed_long, int32_t, %d)
CONVERSION_FN(float_4_byte, float, %f)
CONVERSION_FN(float_8_byte, double, %f)

static const char *tag_type_str_list[] = {
    "no tag specified",
    "unsigned char",
    "string (with an ending zero)",
    "unsigned short (2 bytes)",
    "unsigned long  (4 bytes)",
    "unsigned rational (2 unsinged long)",
    "signed char",
    "byte sequence",
    "signed short",
    "signed long",
    "signed rational (2 signed long)",
    "float, 4 bytes, IEEE format",
    "float, 8 bytes, IEEE format",
    NULL,
};

#define END (tag_type_table){}

struct tag_type_table {
    const char *(*convert)(FILE *fp, uint32_t addr, size_t count);
}tag_type_table[] = {
    { no_tag_specified_fn },
    { unsigned_char_fn },
    { string_fn },
    { unsigned_short_fn },
    { unsigned_long_fn },
    { unsigned_rational_fn },
    { signed_char_fn },
    { byte_sequence_fn },
    { signed_short_fn },
    { signed_long_fn },
    { signed_rational_fn },
    { float_4_byte_fn },
    { float_8_byte_fn },
};

enum tag_type get_tag_type(uint16_t t)
{
    return (enum tag_type)(t);
}

const char *tag_type_conv(FILE *fp, enum tag_type t, uint32_t addr, size_t count)
{
    return tag_type_table[t].convert(fp, addr, count);
}

const char *tag_type_to_field_str(enum tag_type t)
{
    // if this fails then I know that the user(me) fucked it up. :)
    assert("we added or removed an enum" && t > 0 && t < TAG_TYPE_ENUM_COUNT);
    static_assert(ARRAY_SIZE(tag_type_str_list)-1 == TAG_TYPE_ENUM_COUNT,
            "tag_type_str_listt should have the same members as the enum");
    return tag_type_str_list[t];
}

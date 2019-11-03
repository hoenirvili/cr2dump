#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "tag_type.h"

// let it be 8k bytes, should be enough, (IDK to be honst)
#define MAX_BUFFER_SIZE 8192 + 1

static void normalize_string(char *buffer)
{
    int n = strlen(buffer);

    assert (n < MAX_BUFFER_SIZE);
    char *buff = alloca(n * sizeof *buffer);
    if (buff == NULL) {
        printf("alloca() failed");
        exit(1);
    }
    memset(buff, 0, n * sizeof *buffer);
    int last_index_space = -1;
    size_t count = 0;
    for (int i = 0, j = 0; i < n && j < n; i++) {
        if (buffer[i] == '\n')
            continue;

        if (isspace(buffer[i])) {
            if (j - 1 >= 0) {
                if (isspace(buff[j-1]))
                    continue;
            } else
                continue;
        }

        buff[j] = buffer[i];
        //TODO: hanlde newline addition better
        if ((count % 50 == 0) && (j+1 < n)) {
            if (isspace(buff[j])) {
                buff[j] = '\n';
                count = 0;
                continue;
            }
            buff[j+1] = '\n';
            count = 0;
        }

        j++;
        count++;
    }

    memset(buffer, 0, n * sizeof *buffer);
    memcpy(buffer, buff, n * sizeof *buffer);
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// we could alloc memory for ever str conversion but I'm a cheap asshole
static char buffer[MAX_BUFFER_SIZE];

static const char *no_tag_specified_fn(FILE *fp, uint32_t addr, size_t count) { return NULL; }

static const char *unsigned_char_fn(FILE *fp, uint32_t addr, size_t count)
{
    assert(MAX_BUFFER_SIZE > sizeof(unsigned char) * count);
    memset(buffer, 0, sizeof(buffer));
    fseek(fp, addr, SEEK_SET);
    fread(buffer, 1, count, fp);
    rewind(fp);
    if (addr == 0x0000000000b05c) {
        normalize_string(buffer);
    }
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

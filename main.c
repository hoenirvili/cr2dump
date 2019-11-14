#include <string.h>
#include "file.h"
#include "tag_type.h"
#include "ifd.h"
#include "cr2_header.h"
#include "tiff_header.h"

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
    size_t bytes = file_size(cr);
    float mbytes = file_mb(bytes);
    printf("File size: %.4f MB\n", mbytes);

    struct tiff_header tiff_header;
    tiff_header_parse(&tiff_header, cr);
    tiff_header_dump(tiff_header, stdout);

    if (!strncmp(tiff_header.endianness, HEADER_LITTLE_ENDIAN, BYTE_ORDER_CHARS)) {
        puts("\nFile: is little endian format");
    } else if (!strncmp(tiff_header.endianness, HEADER_BIG_ENDIAN, BYTE_ORDER_CHARS)) {
        puts("\nFile is big endian format");
    }

    struct cr2_header cr2_header;
    cr2_header_parse(&cr2_header, cr);
    cr2_header_dump(cr2_header, stdout);

    struct ifd ifds[N_IFDS];
    ifds_parse(ifds, N_IFDS, tiff_header.offset_to_ifd, cr);
    ifds_dump(ifds, N_IFDS, cr, stdout);

    struct ifd exif;
    ifd_parse(&exif, ifd_exif_value(ifds[0]), cr);
    puts("\n\nEXIF");
    ifd_dump(exif, cr, stdout);
    ifd_entries_free(ifds);

    fclose(cr);
}

    /* struct ifd ifds[MAX_NUMBER_OF_IFDS]; */
    /* exif exif_sub_ifd; */
    /* struct maker_note_ifd mnifd; */
    /* parse_all_ifds(cr, tiffhdr.offset_to_ifd, ifds, MAX_NUMBER_OF_IFDS, &exif_sub_ifd, &mnifd); */
    /* dump_ifds(cr, ifds, MAX_NUMBER_OF_IFDS); */
    /* puts("EXIF"); */
    /* dump_ifd(cr, exif_sub_ifd, 0); */
    /* for (size_t i = 0; i < MAX_NUMBER_OF_IFDS; i++ ) */
    /*     free_ifd_entries(ifds[i]); */
    /* free_exif_entries(&exif_sub_ifd); */


/* struct maker_note_ifd { */
/*     struct ifd ifd; */
/*     struct tiff_header hdr; */
/* }; */
/*  */
/*  */
/* #define TAG_ID_MAKER_NOTE 0x927c */
/*  */
/* // exif which consists of the same memory layout as an struct ifd */
/* typedef struct ifd exif; */
/*  */
/* static void free_exif_entries(exif *exif) { free(exif->entries); } */
/*  */
/* // The .CR2 file is based on the TIFF file format */
/* // This TIFF file has 4 Image File Directories (IFDs). */
/*  */
/* static void dump_ifds(FILE *fp, struct ifd *ifds, size_t nfds) */
/* { */
/*     for (int i = 0; i < nfds; i++) { */
/*         puts("\n"); */
/*         dump_ifd(fp, ifds[i], i); */
/*     } */
/* } */
/*  */
/* #define TAG_ID_EXIF 0x8769 */
/*  */
/* static uint16_t ifd_exif_tag_value(const struct ifd *ifd) */
/* { */
/*     for (size_t i = 0; i < ifd->number_of_entries; i++) */
/*         if (ifd->entries[i].tag_id == TAG_ID_EXIF) */
/*             return ifd->entries[i].value; */
/*     return 0; */
/* } */
/*  */
/* static void parse_exif_sub_ifd(const struct ifd *ifd, exif *exif, FILE *fp) */
/* { */
/*     uint16_t value = ifd_exif_tag_value(ifd); */
/*     if (!value) { */
/*         printf("ifd_exif_tag_value() failed"); */
/*         exit(1); */
/*     } */
/*     parse_ifd(fp, value, exif); */
/* } */
/*  */
/* static uint16_t exif_maker_note_value(const exif *exif) */
/* { */
/*     for (size_t i = 0; i < exif->number_of_entries; i++) */
/*         if (exif->entries[i].tag_id == TAG_ID_MAKER_NOTE) */
/*             return exif->entries[i].value; */
/*     return 0; */
/* } */
/*  */
/* static void parse_maker_note_ifd( */
/*         const exif *exif, */
/*         struct maker_note_ifd *mnifd, */
/*         FILE *fp) */
/* { */
/*     uint16_t value = exif_maker_note_value(exif); */
/*     if (!value) { */
/*         printf("ifd_exif_tag_value() failed"); */
/*         exit(1); */
/*     } */
/*     parse_ifd(fp, value, &mnifd->ifd); */
/*     fseek(fp, value + mnifd->ifd.number_of_entries * sizeof(mnifd->ifd.entries), SEEK_SET); */
/*     rewind(fp); */
/* } */
/*  */
/* static void parse_all_ifds( */
/*         FILE *fp, */
/*         unsigned int tiff_offset, */
/*         struct ifd *ifds, */
/*         size_t nfds, */
/*         struct ifd *exif, */
/*         struct maker_note_ifd *mnifd) */
/* { */
/*     for (size_t i = 0; i < nfds; i++) { */
/*         parse_ifd(fp, tiff_offset, &ifds[i]); */
/*         tiff_offset = ifds[i].next_ifd_offset; */
/*         if (i == 0) { */
/*             parse_exif_sub_ifd(&ifds[i], exif, fp); */
/*             //parse_maker_note_ifd(exif, mnifd, fp); */
/*         } */
/*     } */
/* } */


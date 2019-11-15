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

    struct ifd makernote;
    ifd_parse(&makernote, ifd_maker_note(exif), cr);
    puts("\nMAKERNOTE");
    ifd_dump(makernote, cr, stdout);

    ifd_entries_free(exif);
    ifd_entries_free(makernote);
    ifds_entries_free(ifds, N_IFDS);
    fclose(cr);
}

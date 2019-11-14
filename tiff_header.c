#include "tiff_header.h"

void tiff_header_dump(struct tiff_header hdr, FILE *fp)
{
    fprintf(fp, "\nTIFF header\n");
    fprintf(fp, "endianness: %2.2s\n", hdr.endianness);
    fprintf(fp, "magic_number: %#06hx\n", hdr.magic_number);
    fprintf(fp, "offset_to_ifd: %#08x\n", hdr.offset_to_ifd);
}

void tiff_header_parse(struct tiff_header *hdr, FILE *fp)
{
    fread(hdr, sizeof(*hdr), 1, fp);
    rewind(fp);
}

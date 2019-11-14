#include "cr2_header.h"
#include "tiff_header.h"

void cr2_header_dump(struct cr2_header hdr, FILE *fp)
{
    fprintf(fp, "\nCR2 Header\n");
    fprintf(fp, "canon_raw_marker: %2.2s\n", hdr.canon_raw_marker);
    fprintf(fp, "version: %d\n", hdr.version);
    fprintf(fp, "offset_to_raw_ifd: %#08x\n", hdr.offset_to_raw_ifd);
}

void cr2_header_parse(struct cr2_header *hdr, FILE *fp)
{
    fseek(fp, sizeof(struct tiff_header), SEEK_SET);
    fread(hdr, sizeof(*hdr), 1, fp);
    rewind(fp);
}



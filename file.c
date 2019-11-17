#include <sys/stat.h>
#include "file.h"

// Kilobytes in bytes
static const float FILE_KB = 1024.0f;

// Megabytes in bytes
static const float FILE_MB = FILE_KB * 1024.0f;

// fsize returns the file size in bytes
off_t file_size(FILE *fp)
{
    struct stat buf;
    int err = fstat(fileno(fp), &buf);
    if (err < 0)
        return err;
    return buf.st_size;
}

// MB transform bytes to megabytes
float file_mb(off_t bytes ) {
    return bytes / FILE_MB;
}

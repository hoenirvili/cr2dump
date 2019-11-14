#include <sys/stat.h>
#include "file.h"

// Kilobytes in bytes
static const size_t FILE_KB = 1024;

// Megabytes in bytes
static const size_t FILE_MB = FILE_KB * 1024;

// fsize returns the file size in bytes
int file_size(FILE *fp)
{
    struct stat buf;
    int err = fstat(fileno(fp), &buf);
    if (err < 0)
        return err;
    return buf.st_size;
}

// MB transform bytes to megabytes
float file_mb(size_t bytes) {
    return bytes / FILE_MB;
}

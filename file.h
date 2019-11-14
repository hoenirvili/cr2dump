#pragma once

#include <stdio.h>

// fsize returns the file size in bytes
extern int file_size(FILE *fp);

// file_mb transform bytes into MB bytes
extern float file_mb(size_t bytes);

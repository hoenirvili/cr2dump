#pragma once

#include <stdio.h>

// fsize returns the file size in bytes
extern off_t file_size(FILE *fp);

// file_mb transform bytes into MB bytes
extern float file_mb(off_t bytes);

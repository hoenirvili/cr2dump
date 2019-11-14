#pragma once

#include <stdio.h>
#include <stdlib.h>

extern void inline malloc_check(void *p, const char *str)
{
    if (p == NULL) {exit(1); puts(str);}
}

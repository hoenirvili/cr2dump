#pragma once

#include <stdio.h>
#include <stdlib.h>

inline extern void malloc_check(void *p, const char *str)
{
    if (p == NULL) {exit(1); puts(str);}
}

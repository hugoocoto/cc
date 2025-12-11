#ifndef STRING_H_
#define STRING_H_

#include "malloc.h"
#include "stddef.h"

#define strndup strndup
static char *
strndup(const char *src, size_t size)
{
        char *buf = malloc(size + 1);
        size_t i;
        for (i = 0; i < size && src[i]; i++) {
                buf[i] = src[i];
        }
        buf[i] = 0;
        return buf;
}

#endif

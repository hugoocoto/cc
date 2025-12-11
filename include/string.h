#ifndef STRING_H_
#define STRING_H_

#include "malloc.h"
#include "stddef.h"

static size_t
strlen(const char *str)
{
        const char *c = str;
        while (*c)
                ++c;
        return c - str;
}

static int
strncmp(const char *s1, const char *s2, size_t maxlen)
{
        size_t i;
        for (i = 0; i < maxlen && s1[i]; i++) {
                if (s1[i] != s2[i]) return s1[i] - s2[i];
        }
        return 0;
}

static int
strcmp(const char *s1, const char *s2)
{
        return strncmp(s1, s2, sizeof(s1) + 1);
}

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

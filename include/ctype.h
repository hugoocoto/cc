#include "stdbool.h"

static bool
_isspace(char c)
{
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool
_isdigit(char c)
{
        return c >= '0' && c <= '9';
}

static bool
_ishexdigit(char c)
{
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z');
}

static bool
_isnondigit(char c)
{
        return c == '_' ||
               (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z');
}

static bool
_isunichar(char *c)
{
        if (*(c++) != '\\') return false;
        int i;
        switch (*(c++)) {
        case 'u':
                for (i = 0; i < 4; i++)
                        if (!_ishexdigit(c[i])) return false;
                return true;
        case 'U':
                for (i = 0; i < 4; i++)
                        if (!_ishexdigit(c[i])) return false;
                return true;
        }
        return false;
}

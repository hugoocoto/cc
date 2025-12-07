#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static const char *const lex_type_repr_lookup[] = {
#define TOKEN(name) [name] = #name,
#include "tokens.h"
#undef TOKEN
};

const char *
lex_type_repr(LexType type)
{
        if (type < 0 || type >= LexTypeLen) return "LexTypeOOB";
        return lex_type_repr_lookup[type];
}

struct LexDef {
        char *str;
        LexType type;
};

// I know this is slow and inefficient but as I don't know how my language is
// going to looks like, It's easier for me to do it this way.
const struct LexDef lex_def_table[] = {
#define LEXEME(repr, type) { repr, type },
#include "lexemes.h"
#undef LEXEME
};

static void
_report(char *file, int line, char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        printf("[%s:%d] Lexer: ", file, line);
        vprintf(format, ap);
        putchar(10);
        va_end(ap);
}

char
parse_char(char **s)
{
        if (**s == '\\') {
                ++*s;
                switch (**s) {
                case 'a': ++*s; return '\a';
                case 'b': ++*s; return '\b';
                case 'f': ++*s; return '\f';
                case 'n': ++*s; return '\n';
                case 'r': ++*s; return '\r';
                case 't': ++*s; return '\t';
                case 'v': ++*s; return '\v';
                default: break;
                }
        }
        if (**s == '\'') return 0; // allow empty char as null char
        return *((*s)++);
}

char *
parse_string()
{
        return NULL;
}

char *
parse_identifier(char **s, int *len)
{
        char *start = *s;
        *len = 0;
        while (isalnum(**s) || **s == '_') {
                ++*s;
                ++*len;
        }
        return start;
}

long
parse_num(char **s)
{
        long n = 0;

        while (isdigit(**s)) {
                n *= 10;
                n += **s - '0';
                ++*s;

                // Allow '_' separators
                if (**s == '_' && isdigit((*s)[1])) ++*s;
        }
        return n;
}

#define lex_new_any(_s, _line, _offset, _type) \
        do {                                   \
                Lex *lex = lex_new();          \
                lex->as.text = _s;             \
                lex->type = _type;             \
                lex->line = _line;             \
                lex->offset = _offset;         \
                current->next = lex;           \
                current = lex;                 \
        } while (0);

#define lex_new_s(_s, _line, _offset) \
        lex_new_any(_s, _line, _offset, STRING)

#define lex_new_f(_f, _line, _offset)  \
        do {                           \
                Lex *lex = lex_new();  \
                lex->as.f = _f;        \
                lex->type = NUMBER;    \
                lex->line = _line;     \
                lex->offset = _offset; \
                current->next = lex;   \
                current = lex;         \
        } while (0);

#define lex_new_i(_i, _line, _offset)  \
        do {                           \
                Lex *lex = lex_new();  \
                lex->as.i = _i;        \
                lex->type = INTEGER;   \
                lex->line = _line;     \
                lex->offset = _offset; \
                current->next = lex;   \
                current = lex;         \
        } while (0);

#define lex_new_c(_c, _line, _offset)  \
        do {                           \
                Lex *lex = lex_new();  \
                lex->as.c = _c;        \
                lex->type = CHAR;      \
                lex->line = _line;     \
                lex->offset = _offset; \
                current->next = lex;   \
                current = lex;         \
        } while (0);

Lex *
lex_new()
{
        return calloc(1, sizeof(Lex));
}

void
check_tables_integrity()
{
        // check that lexeme-type is a valid token. Throws compile error if not
        const LexType lex_types[] = {
/*         */ #define LEXEME(repr, type) type,
/*         */ #include "lexemes.h"
/*         */ #undef LEXEME
        };
        (void) lex_types;
        return;
}

Lex
lexer(char *s)
{
        check_tables_integrity();

        Lex lex_head = { 0 };
        Lex *current = &lex_head;
        size_t lex_def_table_len = sizeof lex_def_table / sizeof *lex_def_table;
        size_t i;
        size_t len;
        int linecount = 1;
        char *linestart = s;

// as a while loop but without indentation
cont:

        while (isspace(*s)) {
                if (*s == '\n') {
                        ++linecount;
                        linestart = s + 1;
                }
                ++s;
        }

        char *lexoffset = s;
        if (isdigit(*s)) {
                long num = parse_num(&s);
                if (*s == '.' && isdigit(s[1])) {
                        ++s; // consume dot
                        double dec = parse_num(&s);
                        // Waaaay too inacurate
                        dec /= pow(10, (int) log10(dec) + 1);

                        lex_new_f(num + dec, linecount, lexoffset - linestart + 1);

                } else {
                        lex_new_i(num, linecount, lexoffset - linestart + 1);
                }
                goto cont;
        }

        if (*s == '"') {
                int capacity = 8; // initial cap
                char *buf = malloc(capacity);
                int count = 0;
                ++s; // consume "
                while (*s != '"') {
                        if (*s == '\n') break; // error: multiline not supported
                        char c = parse_char(&s);
                        if (count + 1 == capacity) {
                                buf = realloc(buf, (capacity *= 2));
                        }
                        buf[count++] = c;
                }
                buf[count] = 0; // zero terminated
                if (*s != '\"') {
                        report("Invalid string at %d:%d: `%*s`",
                               linecount,
                               lexoffset - linestart + 1,
                               s - lexoffset + 1,
                               lexoffset);
                        goto cont;
                }
                ++s; // consume "
                lex_new_s(buf, linecount, lexoffset - linestart + 1);
                goto cont;
        }

        if (*s == '\'') {
                ++s; // consume '
                char c = parse_char(&s);
                if (*s != '\'') {
                        report("Invalid char at %d:%d: `%*s`",
                               linecount,
                               lexoffset - linestart + 1,
                               s - lexoffset + 1,
                               lexoffset);
                        goto cont;
                }
                ++s; // consume '
                lex_new_c(c, linecount, lexoffset - linestart + 1);
                goto cont;
        }


        for (i = 0; i < lex_def_table_len; i++) {
                len = strlen(lex_def_table[i].str);
                if (!strncmp(lex_def_table[i].str, s, len)) {
                        s += len;

                        lex_new_any(memcpy(calloc(1, len + 1), lex_def_table[i].str, len), linecount, lexoffset - linestart + 1, lex_def_table[i].type);
                        goto cont;
                }
        }

        if (isalpha(*s) || *s == '_') {
                int len;
                char *id = parse_identifier(&s, &len);
                Lex *lex = lex_new();
                lex->as.text = memcpy(calloc(1, len + 1),
                                      id,
                                      len);
                lex->type = IDENTIFIER;
                lex->line = linecount;
                lex->offset = lexoffset - linestart + 1;
                current->next = lex;
                current = lex;
                goto cont;
        }

        if (*s) {
                report("unknown char: `%c`", *s);
                ++s;
                goto cont;
        }

        return lex_head;
}

void
lex_free(Lex head)
{
        Lex *current = head.next;
        Lex *next;
        while (current) {
                next = current->next;
                switch (current->type) {
                case INTEGER:
                case CHAR:
                case NUMBER: break;
                default: free(current->as.text); break;
                }
                free(current);
                current = next;
        }
}

void
lex_print_all(Lex head)
{
        Lex *current = &head;
        while (current->next) {
                lex_print(current->next);
                current = current->next;
        }
}

void
lex_print(Lex *l)
{
        printf("Lex at %d,%d -> %s", l->line, l->offset, lex_type_repr(l->type));

        switch (l->type) {
        case INTEGER: printf(" `%ld`", l->as.i); break;
        case NUMBER: printf(" `%lf`", l->as.f); break;
        case CHAR: printf(isprint(l->as.c) ? " '%c'" : " %d", l->as.c); break;
        case STRING: printf(" \"%s\"", l->as.text); break;
        default: printf(" `%s`", l->as.text); break;
        }
        putchar(10);
}

void
lex_print_groups(Lex head)
{
        // clang-format off
#define OSEP "\033[0;3m" "("
#define CSEP "\033[0;3m" ")" "\033[0m"
#define STYLE "\033[0;1m"
        // clang-format on

        Lex *current = &head;
        while (current->next) {
                switch (current->next->type) {
                case INTEGER:
                        printf(OSEP STYLE "%ld" CSEP,
                               current->next->as.i);
                        break;
                case NUMBER:
                        printf(OSEP STYLE "%lf" CSEP,
                               current->next->as.f);
                        break;
                case CHAR:
                        printf(isprint(current->next->as.c) ?
                               OSEP STYLE "%c'" CSEP :
                               OSEP STYLE "%d'" CSEP,
                               current->next->as.c);
                        break;
                case STRING:
                        printf(OSEP STYLE "%s\"" CSEP,
                               current->next->as.text);
                        break;
                default:
                        printf(OSEP STYLE "%s" CSEP,
                               current->next->as.text);
                        break;
                }
                current = current->next;
        }
        putchar(10);
}

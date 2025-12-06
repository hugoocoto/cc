#ifndef LEXER_H_
#define LEXER_H_

typedef enum {
#define TOKEN(name) name,
#include "tokens.h"
#undef TOKEN
} LexType;

typedef struct Lex {
        union {
                char *text;
                long i;
                double f;
                char c;
        } as;
        LexType type;
        int line, offset;
        struct Lex *next;
} Lex;

#define report(format, ...) _report(__FILE__, __LINE__, format, ##__VA_ARGS__)

Lex lexer(char *s);
void lex_print(Lex *l);
void lex_print_all(Lex head);
void lex_free(Lex head);
const char *lex_type_repr(LexType type);

#endif

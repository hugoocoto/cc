#ifndef PARSER_H_
#define PARSER_H_

#include "lexer.h"
#include "scsvargs.h"

typedef enum Tok_type {
/*   */ #define STMT(name, ...) name,
/*   */ #include "stmts.h"
/*   */ #undef STMT
} Tok_type;

typedef struct Tok {
        Tok_type type;
        union {
                // clang-format off
/*  Crazy    */ #define STMT(name, ...) struct { scsvargs(__VA_ARGS__) } name;
/*   Shit    */ #define Tok struct Tok
/*           */ #include "stmts.h"
/*           */ #undef Tok
/*           */ #undef STMT
                // clang-format on
        };
        struct Tok *next;
} Tok;

Tok parse(Lex head_lex, int *has_error);
void tok_free(Tok head);
void tok_print_all(Tok head);
void tok_print(Tok *t);

#endif

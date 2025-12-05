#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"

typedef struct Tok {
        struct Tok *next;
} Tok;

jmp_buf parse_error_jmpenv; // jump here on parse error
Lex *current;               // current lexeme

Lex *
consume()
{
        Lex *ret = current;
        current = current->next;
        return ret;
}

bool
match(LexType type)
{
        return current->type == type;
}

Lex *
match_and_consume(LexType type)
{
        if (match(type)) {
                return consume();
        }
        return current;
}

Tok
parse(Lex head)
{
        bool has_error = false;
        current = &head; // global current

        if (setjmp(parse_error_jmpenv)) {
                puts("Ups something goes wrong!");
                has_error = true;
        }

        for (; current->next; current = current->next) {
                Tok.next = parse_get_stmt();
        }
}

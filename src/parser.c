#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

jmp_buf parse_error_jmpenv; // jump here on parse error
Lex *current_lex;           // current lexeme

static const char *const tok_type_repr_lookup[] = {
#define TOKEN(a) [a] = #a,
#include "tokens.h"
#undef TOKEN
};

Lex *
consume()
{
        Lex *ret = current_lex;
        current_lex = current_lex->next;
        return ret;
}

#define match(type, ...) match_list(type, ##__VA_ARGS__, NULL)

bool
match_list(LexType type, ...)
{
        Lex *c = current_lex;
        va_list ap;
        va_start(ap, type);
        LexType t = type;
        do {
                if (!current_lex || current_lex->type != type) {
                        current_lex = c;
                        return false;
                }
                current_lex = current_lex->next;
        } while ((t = va_arg(ap, LexType)));
        current_lex = c; // do not consume
        return true;
}

Lex *
match_and_consume(LexType type)
{
        if (match(type)) {
                return consume();
        }
        return current_lex;
}

Tok *
tok_new()
{
        return calloc(1, sizeof(Tok));
}

Tok *
parse_get_stmt()
{
        return tok_new();
}

Tok
parse(Lex head_lex, int *has_error)
{
        Tok head_tok = { 0 };
        Tok *current_tok = &head_tok;
        current_lex = &head_lex; // global current
        *has_error = false;

        if (setjmp(parse_error_jmpenv)) {
                puts("Ups something went wrong!");
                *has_error = true;
        }

        // Iterate over lexemes
        for (; current_lex->next; current_lex = current_lex->next) {
                current_tok->next = parse_get_stmt();
                current_tok = current_tok->next;
        }

        return head_tok;
}

void
tok_free(Tok head_tok)
{
        Tok *current = head_tok.next;
        Tok *next;
        while (current) {
                next = current->next;
                free(current);
                current = next;
        }
}

void
tok_print(Tok *t)
{
        printf("Tok: %p\n", t);
}

void
tok_print_all(Tok head)
{
        Tok *current = &head;
        while (current->next) {
                tok_print(current->next);
                current = current->next;
        }
}

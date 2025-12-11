/* This file is part of the Hugo's C Compiler
 *
 * Copyright (C) 2025  Hugo Coto Flórez
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY of FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the source code. If not, see <https://www.gnu.org/licenses/>
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */

#include "../include/assert.h"
#include "../include/ctype.h"
#include "../include/malloc.h"
#include "../include/stdbool.h"
#include "../include/string.h"


typedef struct Tok {
        int line, offset;
        enum {
                TOK_KEYWORD,
                TOK_CONSTANT,
                TOK_STRING_LITERAL,
                TOK_PUNCTUATOR,
                TOK_IDENTIFIER,
        } type;
        union {
                struct {
                        // clang-format off
                        enum { AUTO, BREAK, CASE, CHAR, CONST, CONTINUE, 
                                DEFAULT, DO, DOUBLE, ELSE, ENUM, EXTERN, FLOAT, 
                                FOR, GOTO, IF, INLINE, INT, LONG, REGISTER, 
                                RESTRICT, RETURN, SHORT, SIGNED, SIZEOF, STATIC, 
                                STRUCT, SWITCH, TYPEDEF, UNION, UNSIGNED, VOID, 
                                VOLATILE, WHILE, _BOOL, _COMPLEX, _IMAGINARY,
                        } type;
                        // clang-format on
                } keyword;
                struct {
                } constant;
                struct {
                } string_literal;
                struct {
                        enum {
                                PUN_NONE = 0,         // invalid or none
                                PUN_OPAR,             // (
                                PUN_CPAR,             // )
                                PUN_OBRACKET,         // [
                                PUN_CBRACKET,         // ]
                                PUN_OBRACE,           // {
                                PUN_CBRACE,           // }
                                PUN_DOT,              // .
                                PUN_ARROW,            // ->
                                PUN_COMMA,            // ,
                                PUN_COLON,            // :
                                PUN_SEMICOLON,        // ;
                                PUN_ELLIPSIS,         // ...
                                PUN_QUESTION,         // ?
                                PUN_PLUS,             // +
                                PUN_PLUSPLUS,         // ++
                                PUN_MINUS,            // -
                                PUN_MINUSMINUS,       // --
                                PUN_STAR,             // *
                                PUN_SLASH,            // /
                                PUN_MOD,              // %
                                PUN_TILDE,            // ~
                                PUN_NOT,              // !
                                PUN_ASSIGN,           // =
                                PUN_PLUSEQ,           // +=
                                PUN_MINUSEQ,          // -=
                                PUN_STAREQ,           // *=
                                PUN_SLASHEQ,          // /=
                                PUN_MODEQ,            // %=
                                PUN_EQ,               // ==
                                PUN_NE,               // !=
                                PUN_LT,               // <
                                PUN_GT,               // >
                                PUN_LTE,              // <=
                                PUN_GTE,              // >=
                                PUN_AND,              // &
                                PUN_OR,               // |
                                PUN_XOR,              // ^
                                PUN_LSHIFT,           // <<
                                PUN_RSHIFT,           // >>
                                PUN_LAND,             // &&
                                PUN_LOR,              // ||
                                PUN_ANDEQ,            // &=
                                PUN_XOREQ,            // ^=
                                PUN_OREQ,             // |=
                                PUN_LSHIFTEQ,         // <<=
                                PUN_RSHIFTEQ,         // >>=
                                PUN_HASH,             // #
                                PUN_PASTE,            // ##
                                PUN_LTCOLON,          // <:
                                PUN_COLONGT,          // :>
                                PUN_LTMOD,            // <%
                                PUN_MODGT,            // %>
                                PUN_MODCOLON,         // %:
                                PUN_MODCOLONMODCOLON, // %:%:
                                PUN_MAXLEN,
                        } type;
                } punctuator;
                struct {
                        char *name;
                } identifier;
        };
        struct Tok *next;
} Tok;

const char *
tok_pun_repr(int pun)
{
        static const char *const lookup[] = {
                [PUN_OPAR] = "(",
                [PUN_CPAR] = ")",
                [PUN_OBRACKET] = "[",
                [PUN_CBRACKET] = "]",
                [PUN_OBRACE] = "{",
                [PUN_CBRACE] = "}",
                [PUN_DOT] = ".",
                [PUN_ARROW] = "->",
                [PUN_COMMA] = ",",
                [PUN_COLON] = ":",
                [PUN_SEMICOLON] = ";",
                [PUN_ELLIPSIS] = "...",
                [PUN_QUESTION] = "?",
                [PUN_PLUS] = "+",
                [PUN_PLUSPLUS] = "++",
                [PUN_MINUS] = "-",
                [PUN_MINUSMINUS] = "--",
                [PUN_STAR] = "*",
                [PUN_SLASH] = "/",
                [PUN_MOD] = "%",
                [PUN_TILDE] = "~",
                [PUN_NOT] = "!",
                [PUN_ASSIGN] = "=",
                [PUN_PLUSEQ] = "+=",
                [PUN_MINUSEQ] = "-=",
                [PUN_STAREQ] = "*=",
                [PUN_SLASHEQ] = "/=",
                [PUN_MODEQ] = "%=",
                [PUN_EQ] = "==",
                [PUN_NE] = "!=",
                [PUN_LT] = "<",
                [PUN_GT] = ">",
                [PUN_LTE] = "<=",
                [PUN_GTE] = ">=",
                [PUN_AND] = "&",
                [PUN_OR] = "|",
                [PUN_XOR] = "^",
                [PUN_LSHIFT] = "<<",
                [PUN_RSHIFT] = ">>",
                [PUN_LAND] = "&&",
                [PUN_LOR] = "||",
                [PUN_ANDEQ] = "&=",
                [PUN_XOREQ] = "^=",
                [PUN_OREQ] = "|=",
                [PUN_LSHIFTEQ] = "<<=",
                [PUN_RSHIFTEQ] = ">>=",
                [PUN_HASH] = "#",
                [PUN_PASTE] = "##",
                [PUN_LTCOLON] = "<:",
                [PUN_COLONGT] = ":>",
                [PUN_LTMOD] = "<%",
                [PUN_MODGT] = "%>",
                [PUN_MODCOLON] = "%:",
                [PUN_MODCOLONMODCOLON] = "%:%:",
        };
        if (sizeof lookup / sizeof lookup[0] != PUN_MAXLEN - PUN_NONE) {
                printf("Lookup size: %ld, with PUN_MAXLEN = %d\n",
                       sizeof lookup / sizeof lookup[0], PUN_MAXLEN);
                exit(1);
        }
        if (pun <= PUN_NONE || pun >= PUN_MAXLEN) {
                printf("Invalid pun: %d of %d\n", pun, PUN_MAXLEN);
                exit(1);
        }
        return lookup[pun];
}

Tok *
tok_new()
{
        return calloc(1, sizeof(Tok));
}

bool
match_consume_identifier(char **cursor, Tok *prev)
{
        char *start = *cursor;
        while (1) {
                if (_isnondigit(**cursor) ||
                    (_isdigit(**cursor) && start != *cursor)) {
                        ++*cursor;
                        continue;
                }

                if (_isunichar(*cursor)) {
                        switch ((*cursor)[1]) {
                        case 'u':
                                *cursor += 4;
                                continue;
                        case 'U':
                                *cursor += 8;
                                continue;
                        default:
                                assert(!"Non reachable region reached!");
                        }
                }
                break; // current char doesn't match anything
        }

        if (start == *cursor) {
                // The cursor wasn't pointing to a identifier
                return false;
        }

        prev->next = tok_new();
        prev->next->type = TOK_IDENTIFIER;
        prev->next->identifier.name = strndup(start, *cursor - start);
        printf("New identifier: `%s`\n", prev->next->identifier.name);
        return true;
}

bool
match_consume_punctuator(char **cursor, Tok *prev)
{
        int p = PUN_NONE;

        switch (**cursor) {
        case '!':
                if ((*cursor)[1] == '=') {
                        p = PUN_NE;
                        ++*cursor;
                } else
                        p = PUN_NOT;
                break;

        case '#':
                if ((*cursor)[1] == '#') {
                        p = PUN_PASTE;
                        ++*cursor;
                } else
                        p = PUN_HASH;
                break;

        case '%':
                if ((*cursor)[1] == ':' &&
                    (*cursor)[2] == '%' &&
                    (*cursor)[3] == ':') {
                        p = PUN_MODCOLONMODCOLON;
                        *cursor += 3;
                } else if ((*cursor)[1] == ':') {
                        p = PUN_MODCOLON;
                        ++*cursor;
                } else if ((*cursor)[1] == '=') {
                        p = PUN_MODEQ;
                        ++*cursor;
                } else if ((*cursor)[1] == '>') {
                        p = PUN_MODGT;
                        ++*cursor;
                } else
                        p = PUN_MOD;
                break;

        case '&':
                if ((*cursor)[1] == '&') {
                        p = PUN_LAND;
                        ++*cursor;
                } else if ((*cursor)[1] == '=') {
                        p = PUN_ANDEQ;
                        ++*cursor;
                } else
                        p = PUN_AND;
                break;

        case '(': p = PUN_OPAR; break;
        case ')': p = PUN_CPAR; break;

        case '*':
                if ((*cursor)[1] == '=') {
                        p = PUN_STAREQ;
                        ++*cursor;
                } else
                        p = PUN_STAR;
                break;

        case '+':
                if ((*cursor)[1] == '=') {
                        p = PUN_PLUSEQ;
                        ++*cursor;
                } else if ((*cursor)[1] == '+') {
                        p = PUN_PLUSPLUS;
                        ++*cursor;
                } else
                        p = PUN_PLUS;
                break;

        case ',':
                p = PUN_COMMA;
                break;

        case '-':
                if ((*cursor)[1] == '-') {
                        p = PUN_MINUSMINUS;
                        ++*cursor;
                } else if ((*cursor)[1] == '=') {
                        p = PUN_MINUSEQ;
                        ++*cursor;
                } else if ((*cursor)[1] == '>') {
                        p = PUN_ARROW;
                        ++*cursor;
                } else
                        p = PUN_MINUS;
                break;

        case '.':
                if ((*cursor)[1] == '.' &&
                    (*cursor)[2] == '.') {
                        p = PUN_ELLIPSIS;
                        ++*cursor;
                } else
                        p = PUN_DOT;
                break;

        case '/':
                if ((*cursor)[1] == '=') {
                        p = PUN_SLASHEQ;
                        ++*cursor;
                } else
                        p = PUN_SLASH;
                break;

        case ':':
                if ((*cursor)[1] == '>') {
                        p = PUN_COLONGT;
                        ++*cursor;
                } else
                        p = PUN_COLON;
                break;

        case ';':
                p = PUN_SEMICOLON;
                break;

        case '<':
                if ((*cursor)[1] == '%') {
                        p = PUN_LTMOD;
                        ++*cursor;
                } else if ((*cursor)[1] == ':') {
                        p = PUN_LTCOLON;
                        ++*cursor;
                } else if ((*cursor)[1] == '<' &&
                           (*cursor)[2] == '=') {
                        p = PUN_LSHIFTEQ;
                        *cursor += 2;
                } else if ((*cursor)[1] == '<') {
                        p = PUN_LSHIFT;
                        ++*cursor;
                } else if ((*cursor)[1] == '=') {
                        p = PUN_LTE;
                        ++*cursor;
                } else
                        p = PUN_LT;
                break;

        case '=':
                if ((*cursor)[1] == '=') {
                        p = PUN_EQ;
                        ++*cursor;
                } else
                        p = PUN_ASSIGN;
                break;

        case '>':
                if ((*cursor)[1] == '>' &&
                    (*cursor)[2] == '=') {
                        p = PUN_RSHIFTEQ;
                        *cursor += 2;
                } else if ((*cursor)[1] == '>') {
                        p = PUN_RSHIFT;
                        ++*cursor;
                } else if ((*cursor)[1] == '=') {
                        p = PUN_GTE;
                        ++*cursor;
                } else
                        p = PUN_GT;
                break;

        case '?':
                p = PUN_QUESTION;
                break;
        case '[':
                p = PUN_OBRACKET;
                break;
        case ']':
                p = PUN_CBRACKET;
                break;

        case '^':
                if ((*cursor)[1] == '=') {
                        p = PUN_XOREQ;
                        ++*cursor;
                } else
                        p = PUN_XOR;
                break;

        case '{':
                p = PUN_OBRACE;
                break;

        case '|':
                if ((*cursor)[1] == '=') {
                        p = PUN_OREQ;
                        ++*cursor;
                } else if ((*cursor)[1] == '|') {
                        p = PUN_LOR;
                        ++*cursor;
                } else
                        p = PUN_OR;
                break;

        case '}':
                p = PUN_CBRACE;
                break;
        case '~':
                p = PUN_TILDE;
                break;
        }

        if (p == PUN_NONE) {
                return false;
        }

        ++*cursor;
        prev->next = tok_new();
        prev->next->type = TOK_PUNCTUATOR;
        prev->next->punctuator.type = p;
        printf("New punctuator: %s\n", tok_pun_repr(p));

        return true;
}

Tok
lexer(char *cursor)
{
        Tok head = { 0 };
        Tok *last = &head;
        int line = 1;
        int offset;
        char *linestart = cursor;

        printf("Fragment: `%s`\n", cursor);

        while (*cursor) {
                while (_isspace(*cursor)) {
                        if (*cursor == '\n') {
                                ++line;
                                /* Can be cursor +1 but this way
                                 * it starts in 1 by default */
                                linestart = cursor;
                        }
                        ++cursor;
                }

                offset = cursor - linestart;

                if (match_consume_punctuator(&cursor, last)) {
                        last->next->line = line;
                        last->next->offset = offset;
                        last = last->next;
                        continue;
                }

                if (match_consume_identifier(&cursor, last)) {
                        last->next->line = line;
                        last->next->offset = offset;
                        last = last->next;
                        continue;
                }

                if (*cursor) {
                        printf("Lex: invalid char: '%c'\n", *cursor);
                        ++cursor;
                }
        }

        return head;
}

void
tok_print(Tok *t)
{
        printf("Token at %d:%d: ", t->line, t->offset);
        switch (t->type) {
        case TOK_PUNCTUATOR:
                printf("Punctuator: `%s`\n", tok_pun_repr(t->punctuator.type));
                break;
        case TOK_IDENTIFIER:
                printf("Identifier: `%s`\n", t->identifier.name);
                break;
        case TOK_KEYWORD:
        case TOK_CONSTANT:
        case TOK_STRING_LITERAL:
        default:
                assert(!"Invalid token type found!");
        }
}

void
tok_printall(Tok head)
{
        Tok *current = &head;
        while (current->next) {
                tok_print(current->next);
                current = current->next;
        }
}

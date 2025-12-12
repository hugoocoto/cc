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
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

jmp_buf syntax_error_jmp_env;

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
                        enum { 
                                KEYWORD_NONE = 0,
                                AUTO, BREAK, CASE, CHAR, CONST, CONTINUE, 
                                DEFAULT, DO, DOUBLE, ELSE, ENUM, EXTERN, FLOAT, 
                                FOR, GOTO, IF, INLINE, INT, LONG, REGISTER, 
                                RESTRICT, RETURN, SHORT, SIGNED, SIZEOF, STATIC, 
                                STRUCT, SWITCH, TYPEDEF, UNION, UNSIGNED, VOID, 
                                VOLATILE, WHILE, _BOOL, _COMPLEX, _IMAGINARY,
                                KEYWORD_MAXLEN,
                        } type;
                        // clang-format on
                } keyword;
                struct {
                } constant;
                struct {
                        char *lit;
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
tok_keyword_repr(int kw)
{
        static const char *const lookup[] = {
                [AUTO] = "AUTO",
                [BREAK] = "BREAK",
                [CASE] = "CASE",
                [CHAR] = "CHAR",
                [CONST] = "CONST",
                [CONTINUE] = "CONTINUE",
                [DEFAULT] = "DEFAULT",
                [DO] = "DO",
                [DOUBLE] = "DOUBLE",
                [ELSE] = "ELSE",
                [ENUM] = "ENUM",
                [EXTERN] = "EXTERN",
                [FLOAT] = "FLOAT",
                [FOR] = "FOR",
                [GOTO] = "GOTO",
                [IF] = "IF",
                [INLINE] = "INLINE",
                [INT] = "INT",
                [LONG] = "LONG",
                [REGISTER] = "REGISTER",
                [RESTRICT] = "RESTRICT",
                [RETURN] = "RETURN",
                [SHORT] = "SHORT",
                [SIGNED] = "SIGNED",
                [SIZEOF] = "SIZEOF",
                [STATIC] = "STATIC",
                [STRUCT] = "STRUCT",
                [SWITCH] = "SWITCH",
                [TYPEDEF] = "TYPEDEF",
                [UNION] = "UNION",
                [UNSIGNED] = "UNSIGNED",
                [VOID] = "VOID",
                [VOLATILE] = "VOLATILE",
                [WHILE] = "WHILE",
                [_BOOL] = "_BOOL",
                [_COMPLEX] = "_COMPLEX",
                [_IMAGINARY] = "_IMAGINARY",

        };
        if (sizeof lookup / sizeof lookup[0] != KEYWORD_MAXLEN - KEYWORD_NONE) {
                printf("Lookup size: %ld, with KEYWORD_MAXLEN = %d\n",
                       sizeof lookup / sizeof lookup[0], KEYWORD_MAXLEN);
                exit(1);
        }
        if (kw <= KEYWORD_NONE || kw >= KEYWORD_MAXLEN) {
                printf("Invalid keyword: %d of %d\n", kw, KEYWORD_MAXLEN);
                exit(1);
        }
        return lookup[kw];
}

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

void
report_error(const char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        printf(__FILE__": Error: ");
        vprintf(format, ap);
        longjmp(syntax_error_jmp_env, 1);
        va_end(ap);
}

Tok *
tok_new()
{
        return calloc(1, sizeof(Tok));
}

bool
match(char **cursor, char c)
{
        if (**cursor == c) {
                ++*cursor;
                return true;
        }
        return false;
}

bool
consume_expect(char **cursor, char c)
{
        if (**cursor == c) {
                ++*cursor;
                return true;
        }
        return false;
}

bool
match_consume_string_literal(char **cursor, Tok *prev)
{
        char *start = *cursor;
        bool is_lstring = false;

        if (match(cursor, 'L')) {
                is_lstring = true;
        }

        if (!consume_expect(cursor, '"')) {
                *cursor = start;
                return false;
        }

        while (1) {
                if (_isschar(**cursor)) {
                        ++*cursor;
                        continue;
                }
                if (**cursor == '\\' && (*cursor)[1] == 'n') {
                        *cursor += 2;
                        continue;
                }
                break;
        }

        if (!consume_expect(cursor, '"')) {
                report_error(' ' <= **cursor && (unsigned char) **cursor <= 127 ?
                             "Expected `\"` but `%c` found\n" :
                             "Expected `\"` but char `0x%0X` found\n",
                             **cursor);
                return false;
        }

        prev->next = tok_new();
        prev->next->type = TOK_STRING_LITERAL;
        prev->next->string_literal.lit =
        strndup(start + is_lstring + 1,
                *cursor - (start + is_lstring + 1 + 1));
        return true;
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
        return true;
}

bool
match_consume_keyword(char **cursor, Tok *prev)
{
        int k;

#define MATCH_CONSUME(_kw_, _KW_)          \
        else if (!strcmp((_kw_), *cursor)) \
        {                                  \
                k = (_KW_);                \
                *cursor += strlen(_kw_);   \
        }

        if (0) {
        }
        MATCH_CONSUME("break", BREAK)
        MATCH_CONSUME("case", CASE)
        MATCH_CONSUME("char", CHAR)
        MATCH_CONSUME("const", CONST)
        MATCH_CONSUME("continue", CONTINUE)
        MATCH_CONSUME("default", DEFAULT)
        MATCH_CONSUME("do", DO)
        MATCH_CONSUME("double", DOUBLE)
        MATCH_CONSUME("else", ELSE)
        MATCH_CONSUME("enum", ENUM)
        MATCH_CONSUME("extern", EXTERN)
        MATCH_CONSUME("float", FLOAT)
        MATCH_CONSUME("for", FOR)
        MATCH_CONSUME("goto", GOTO)
        MATCH_CONSUME("if", IF)
        MATCH_CONSUME("inline", INLINE)
        MATCH_CONSUME("int", INT)
        MATCH_CONSUME("long", LONG)
        MATCH_CONSUME("register", REGISTER)
        MATCH_CONSUME("restrict", RESTRICT)
        MATCH_CONSUME("return", RETURN)
        MATCH_CONSUME("short", SHORT)
        MATCH_CONSUME("signed", SIGNED)
        MATCH_CONSUME("sizeof", SIZEOF)
        MATCH_CONSUME("static", STATIC)
        MATCH_CONSUME("struct", STRUCT)
        MATCH_CONSUME("switch", SWITCH)
        MATCH_CONSUME("typedef", TYPEDEF)
        MATCH_CONSUME("union", UNION)
        MATCH_CONSUME("unsigned", UNSIGNED)
        MATCH_CONSUME("void", VOID)
        MATCH_CONSUME("volatile", VOLATILE)
        MATCH_CONSUME("while", WHILE)
        MATCH_CONSUME("_Bool", _BOOL)
        MATCH_CONSUME("_Complex", _COMPLEX)
        MATCH_CONSUME("_Imaginary", _IMAGINARY)
        else
        {
                return false;
        }

        prev->next = tok_new();
        prev->next->type = TOK_KEYWORD;
        prev->next->keyword.type = k;
        return true;

#undef MATCH_CONSUME
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

        return true;
}

Tok
lexer(char *cursor)
{
        Tok head = { 0 };
        Tok *last = &head;
        int line = 1;
        int offset = 0;
        char *linestart = cursor;
        bool recovered = true;

        if (setjmp(syntax_error_jmp_env) && recovered) {
                printf(" %4d   %*.*s\n",
                       line,
                       (int) strcspn(linestart, "\n\0"),
                       (int) strcspn(linestart, "\n\0"),
                       linestart);
                printf("        %*.*s%s\n",
                       (int) (cursor - linestart),
                       (int) (cursor - linestart),
                       "", "^");
                recovered = false;

                // At least for strings
                while (*cursor != '"' && *cursor != '\n') {
                        ++cursor;
                }
                ++cursor;
        };

        while (*cursor) {
                while (_isspace(*cursor)) {
                        if (*cursor == '\n') {
                                ++line;
                                linestart = cursor + 1;
                        }
                        ++cursor;
                }

                offset = cursor - linestart + 1;

                if (match_consume_punctuator(&cursor, last)) {
                        last->next->line = line;
                        last->next->offset = offset;
                        last = last->next;
                        recovered = true;
                        continue;
                }

                if (match_consume_keyword(&cursor, last)) {
                        last->next->line = line;
                        last->next->offset = offset;
                        last = last->next;
                        recovered = true;
                        continue;
                }

                if (match_consume_string_literal(&cursor, last)) {
                        last->next->line = line;
                        last->next->offset = offset;
                        last = last->next;
                        recovered = true;
                        continue;
                }

                if (match_consume_identifier(&cursor, last)) {
                        last->next->line = line;
                        last->next->offset = offset;
                        last = last->next;
                        recovered = true;
                        continue;
                }

                if (*cursor) {
                        report_error("Invalid char %c\n", *cursor);
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
                printf("keyword: `%s`\n", tok_keyword_repr(t->keyword.type));
                break;
        case TOK_STRING_LITERAL:
                printf("string-literal: `%s`\n", t->string_literal.lit);
                break;
        case TOK_CONSTANT:
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

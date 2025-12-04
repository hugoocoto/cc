#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
        LPAREN = 0,  // (
        RPAREN,      // )
        LBRACKET,    // [
        RBRACKET,    // ]
        LBRACE,      // {
        RBRACE,      // }
        EQUAL,       // =
        STAR,        // *
        PLUS,        // +
        MINUS,       // -
        PIPE,        // |
        AMPERSAND,   // &
        AND,         // &&
        OR,          // ||
        BANG,        // !
        EQUAL_EQUAL, // ==
        NOT_EQUAL,   // !=
        GREATER,     // >
        LESS,        // <
        GREATER_EQ,  // >=
        LESS_EQ,     // <=
        BACKSLASH,
        QUESTION,   // ?
        CARET,      // ^
        QUOTE,      // "
        COMMA,      // ,
        AT,         // @
        HASH,       // #
        UNDERSCORE, // _
        DOLLAR,     // $
        DOT,        // .
        BACKTICK,   // `
        TILDE,      // ~
        PERCENT,    // %
        SLASH,      // /
        DEL,        // DEL
        INTEGER,    // 0
        NUMBER,     // 0.0
        APOSTROPHE, // '
        COLON,      // :
        SEMICOLON,  // ;
        IDENTIFIER,
        KEYWORD,
        LexTypeLen,
} LexType;

const char *const lex_type_repr_lookup[] = {
        [LPAREN] = "LPAREN",
        [RPAREN] = "RPAREN",
        [LBRACKET] = "LBRACKET",
        [RBRACKET] = "RBRACKET",
        [LBRACE] = "LBRACE",
        [RBRACE] = "RBRACE",
        [EQUAL] = "EQUAL",
        [STAR] = "STAR",
        [PLUS] = "PLUS",
        [MINUS] = "MINUS",
        [PIPE] = "PIPE",
        [AMPERSAND] = "AMPERSAND",
        [AND] = "AND",
        [OR] = "OR",
        [BANG] = "BANG",
        [EQUAL_EQUAL] = "EQUAL_EQUAL",
        [NOT_EQUAL] = "NOT_EQUAL",
        [GREATER] = "GREATER",
        [LESS] = "LESS",
        [GREATER_EQ] = "GREATER_EQ",
        [LESS_EQ] = "LESS_EQ",
        [BACKSLASH] = "BACKSLASH",
        [QUESTION] = "QUESTION",
        [CARET] = "CARET",
        [QUOTE] = "QUOTE",
        [COMMA] = "COMMA",
        [AT] = "AT",
        [HASH] = "HASH",
        [UNDERSCORE] = "UNDERSCORE",
        [DOLLAR] = "DOLLAR",
        [DOT] = "DOT",
        [BACKTICK] = "BACKTICK",
        [TILDE] = "TILDE",
        [PERCENT] = "PERCENT",
        [SLASH] = "SLASH",
        [DEL] = "DEL",
        [INTEGER] = "INTEGER",
        [NUMBER] = "NUMBER",
        [APOSTROPHE] = "APOSTROPHE",
        [COLON] = "COLON",
        [SEMICOLON] = "SEMICOLON",
        [IDENTIFIER] = "IDENTIFIER",
        [KEYWORD] = "KEYWORD",
        [LexTypeLen] = "LexTypeLen",
};

const char *
lex_type_repr(LexType type)
{
        if (type < 0 || type >= LexTypeLen) {
                return "LexTypeOOB";
        }
        return lex_type_repr_lookup[type];
}

struct LexDef {
        char *str;
        LexType type;
};

// I know this is slow and inefficient but as I don't know how my language is
// going to looks like, It's easier for me to do it this way.
const struct LexDef lex_def_table[] = {
        { "&&", AND },
        { "||", OR },
        { "==", EQUAL_EQUAL },
        { "!=", NOT_EQUAL },
        { ">=", GREATER_EQ },
        { "<=", LESS_EQ },
        { "\\", BACKSLASH },
        { "(", LPAREN },
        { ")", RPAREN },
        { "[", LBRACKET },
        { "]", RBRACKET },
        { "{", LBRACE },
        { "}", RBRACE },
        { "=", EQUAL },
        { "*", STAR },
        { "+", PLUS },
        { "-", MINUS },
        { "|", PIPE },
        { "&", AMPERSAND },
        { "!", BANG },
        { ">", GREATER },
        { "<", LESS },
        { "?", QUESTION },
        { "^", CARET },
        { "\"", QUOTE },
        { ",", COMMA },
        { "@", AT },
        { "#", HASH },
        { "_", UNDERSCORE },
        { "$", DOLLAR },
        { ".", DOT },
        { "`", BACKTICK },
        { "~", TILDE },
        { "%", PERCENT },
        { "/", SLASH },
        { "'", APOSTROPHE },
        { ":", COLON },
        { ";", SEMICOLON },

        // keywords
        { "if", KEYWORD },
        { "else", KEYWORD },
        { "while", KEYWORD },
        { "for", KEYWORD },
        { "loop", KEYWORD },
        /* ... */
};

typedef struct Lex {
        union {
                char *text;
                long i;
                double f;
        } as;
        LexType type;
        struct Lex *next;
} Lex;

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
#define report(format, ...) _report(__FILE__, __LINE__, format, ##__VA_ARGS__)

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

Lex *
lex_new()
{
        return calloc(1, sizeof(Lex));
}

Lex
lexer(char *s)
{
        Lex lex_head = { 0 };
        Lex *current = &lex_head;
        size_t lex_def_table_len = sizeof lex_def_table / sizeof *lex_def_table;
        int i;
        size_t len;

// as a while loop but without indentation
cont:

        while (isspace(*s)) {
                ++s;
        }

        if (isdigit(*s)) {
                long num = parse_num(&s);
                if (*s == '.' && isdigit(s[1])) {
                        ++s; // consume dot
                        // Waaaay too inacurate
                        double dec = parse_num(&s);
                        dec /= pow(10, (int) log10(dec) + 1);

                        Lex *lex = lex_new();
                        lex->as.f = num + dec;
                        lex->type = NUMBER;
                        current->next = lex;
                        current = lex;

                } else {
                        Lex *lex = lex_new();
                        lex->as.i = num;
                        lex->type = INTEGER;
                        current->next = lex;
                        current = lex;
                }
                goto cont;
        }

        for (i = 0; i < lex_def_table_len; i++) {
                len = strlen(lex_def_table[i].str);
                if (!strncmp(lex_def_table[i].str, s, len)) {
                        s += len;

                        Lex *lex = lex_new();
                        lex->as.text = memcpy(calloc(1, len + 1),
                                              lex_def_table[i].str,
                                              len);
                        lex->type = lex_def_table[i].type;
                        current->next = lex;
                        current = lex;
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
                case NUMBER: break;
                default: free(current->as.text); break;
                }
                free(current);
                current = next;
        }
}

void
lex_print(Lex head)
{
        Lex *current = &head;
        while (current->next) {
                printf("Lex: %s", lex_type_repr(current->next->type));

                switch (current->next->type) {
                case INTEGER: printf(" `%ld`", current->next->as.i); break;
                case NUMBER: printf(" `%lf`", current->next->as.f); break;
                case IDENTIFIER:
                case KEYWORD:
                default:
                        printf(" `%s`", current->next->as.text);
                        break;
                }
                putchar(10);

                current = current->next;
        }
}

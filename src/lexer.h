#ifndef LEXER_H_
#define LEXER_H_

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
        BACKSLASH,   // '\'
        QUESTION,    // ?
        CARET,       // ^
        QUOTE,       // "
        COMMA,       // ,
        AT,          // @
        HASH,        // #
        UNDERSCORE,  // _
        DOLLAR,      // $
        DOT,         // .
        BACKTICK,    // `
        TILDE,       // ~
        PERCENT,     // %
        SLASH,       // /
        DEL,         // DEL
        INTEGER,     // 0
        NUMBER,      // 0.0
        APOSTROPHE,  // '
        COLON,       // :
        SEMICOLON,   // ;
        IDENTIFIER,
        KEYWORD,
        LexTypeLen,
} LexType;

typedef struct Lex {
        union {
                char *text;
                long i;
                double f;
        } as;
        LexType type;
        int line, offset;
        struct Lex *next;
} Lex;

#define report(format, ...) _report(__FILE__, __LINE__, format, ##__VA_ARGS__)

Lex lexer(char *s);
void lex_print(Lex head);
void lex_free(Lex head);
const char *lex_type_repr(LexType type);

#endif

#include "lexer.c"

int
main()
{
        char buff[] = "var 1.65 = sum(1, 0.65);";
        Lex head = lexer(buff);
        lex_print(head);
        lex_free(head);
}

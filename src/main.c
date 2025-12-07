#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "parser.h"

int
main()
{
        char buf[1024];
        int has_error;
        int fd;
        char *filename = "example";

        fd = open(filename, O_RDONLY);
        if (fd < 0) {
                printf("Couldn't open file %s\n", filename);
                return 1;
        }

        while (read(fd, buf, sizeof buf) > 0) {
                Lex head_lex = lexer(buf);
                lex_print_all(head_lex);
                lex_print_groups(head_lex);
                Tok head_tok = parse(head_lex, &has_error);
                // if (!has_error) tok_print_all(head_tok);
                tok_free(head_tok);
                lex_free(head_lex);
        }
        return 1;
}

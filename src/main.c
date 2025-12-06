#include "lexer.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main()
{
        char buf[1024];
        int fd;
        char *filename = "example";
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
                printf("Couldn't open file %s\n", filename);
                return 1;
        }
        while (read(fd, buf, sizeof buf) > 0) {
                Lex head = lexer(buf);
                lex_print_all(head);
                lex_free(head);
        }
        return 1;
}

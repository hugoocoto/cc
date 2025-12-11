#include "lexer.c"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        if (argc != 2) {
                printf("Usage: %s FILE\n", argv[0]);
                return 1;
        }

        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) {
                printf("Error: Can not open %s\n", argv[1]);
                return 1;
        }

        char buf[1024] = { 0 };

        if (read(fd, buf, sizeof buf - 1) < 0) {
                printf("Read error!\n");
        }

        Tok tokens = lexer(buf);
        tok_printall(tokens);
}

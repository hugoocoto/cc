main: obj/main.o obj/lexer.o
	@mkdir -p bin
	gcc obj/main.o obj/lexer.o -o bin/main -lm -fsanitize=address,null -ggdb

obj/main.o: src/main.c
	@mkdir -p obj
	gcc src/main.c -c -o obj/main.o -lm -fsanitize=address,null -ggdb

obj/lexer.o: src/lexer.c
	@mkdir -p obj
	gcc src/lexer.c -c -o obj/lexer.o -lm -fsanitize=address,null -ggdb

clean:
	rm obj bin -r

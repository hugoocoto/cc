FLAGS = -Wall -Wextra -ggdb -fsanitize=address,null -lm

main: bin obj/main.o obj/lexer.o obj/parser.o
	gcc obj/main.o obj/lexer.o obj/parser.o -o bin/main $(FLAGS)

obj/main.o: obj src/main.c
	gcc src/main.c -c -o obj/main.o $(FLAGS) 

obj/lexer.o: obj src/lexer.c src/lexemes.h src/tokens.h
	gcc src/lexer.c -c -o obj/lexer.o $(FLAGS) 

obj/parser.o: obj src/parser.c src/lexemes.h src/tokens.h src/stmts.h
	gcc src/parser.c -c -o obj/parser.o $(FLAGS) 

clean:
	rm obj bin -r

obj:
	mkdir -p obj
bin:
	mkdir -p bin

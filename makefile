FLAGS = -std=gnu99 -Wall -Wextra -ggdb -fsanitize=address,null -lm

main: bin obj/main.o obj/lexer.o
	gcc obj/main.o obj/lexer.o -o bin/main $(FLAGS)

obj/main.o: obj src/main.c
	gcc src/main.c -c -o obj/main.o $(FLAGS) 

obj/lexer.o: obj src/lexer.c
	gcc src/lexer.c -c -o obj/lexer.o $(FLAGS) 

clean:
	rm obj bin -r

obj:
	mkdir -p obj
bin:
	mkdir -p bin

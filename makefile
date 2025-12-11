FLAGS = -Wall -Wextra -ggdb 
# -fsanitize=address,null -lm

main: bin src/*.c
	gcc src/main.c -o bin/main $(FLAGS)

clean:
	rm obj bin -r

obj:
	mkdir -p obj
bin:
	mkdir -p bin

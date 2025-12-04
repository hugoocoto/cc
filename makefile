main: src/main.c
	@mkdir -p bin
	gcc src/main.c -o bin/main -lm -fsanitize=address,null

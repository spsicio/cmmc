CC := gcc
CFLAGS := -std=c99

main: main.c lexer.h lexer.c ast.h ast.c parser.h parser.c test.c test.h
	$(CC) $(CFLAGS) main.c lexer.c ast.c parser.c test.c -o main

clean:
	rm main

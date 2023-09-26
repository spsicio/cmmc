CC := gcc
CFLAGS := -std=c99

main: main.c parser.c parser.h lexer.c lexer.h
	$(CC) $(CFLAGS) main.c parser.c lexer.c -o main

clean:
	rm main

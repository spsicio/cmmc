CC := gcc
CFLAGS := -std=c99

parser: main.c lexer.h lexer.c ast.h ast.c parser.h parser.c test.c test.h
	$(CC) $(CFLAGS) main.c lexer.c ast.c parser.c test.c -o parser

clean:
	-rm parser

submit:
	-rm ../211502020.zip
	zip -r ../211502020.zip ../Code ../README.md ../report.pdf


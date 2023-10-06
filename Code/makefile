CC := gcc
CFLAGS := -std=c99

objects := main.o tester.o lexer.o ast.o parser.o

parser: $(objects)
	$(CC) $(CFLAGS) -o parser $(objects)

main.o: lexer.h ast.h parser.h tester.h
tester.o: lexer.h ast.h parser.h tester.h
lexer.o: lexer.h
ast.o: lexer.h ast.h
parser.o: lexer.h ast.h parser.h

.PHONY: clean submit
clean:
	-rm parser $(objects)

submit:
	-rm ../211502020.zip
	zip -r ../211502020.zip ../Code ../README.md ../report.pdf


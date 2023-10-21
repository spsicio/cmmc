CC := gcc
CFLAGS := -std=c99

objects := main.o tester.o lexer.o cst.o parser.o \
	   type.o avl.o symtab.o

parser: $(objects)
	$(CC) $(CFLAGS) -o parser $(objects)

main.o: tester.h lexer.h cst.h parser.h type.h avl.h symtab.h
tester.o: tester.h lexer.h cst.h parser.h type.h avl.h symtab.h
lexer.o: lexer.h
cst.o: lexer.h cst.h
parser.o: lexer.h cst.h parser.h
type.o: type.h
avl.o: lexer.h avl.h
symtab.o: lexer.h avl.h symtab.h

.PHONY: clean submit
clean:
	-rm parser $(objects)

submit:
	-rm ../211502020.zip
	zip -r ../211502020.zip ../Code/*.c ../README.md ../report.pdf


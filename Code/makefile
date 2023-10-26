CC := gcc
CFLAGS := -std=c99

header1 := lexer.h cst.h parser.h
header2 := type.h ast.h avl.h symtab.h semantic.h
objects := main.o tester.o lexer.o cst.o parser.o \
	   type.o ast.o avl.o symtab.o semantic.o

parser: $(objects)
	$(CC) $(CFLAGS) -o parser $(objects)

main.o: util.h tester.h $(header1) $(header2)
tester.o: util.h tester.h $(headerr1) $(header2) 
lexer.o: lexer.h
cst.o: lexer.h cst.h
parser.o: $(header1)
type.o: type.h
ast.o: util.h lexer.h cst.h type.h ast.h
avl.o: lexer.h avl.h
symtab.o: lexer.h type.h avl.h symtab.h
semantic.o: util.h lexer.h cst.h $(header2)

.PHONY: clean submit local_test
clean:
	-rm parser $(objects)

submit:
	-rm ../211502020.zip
	zip -r ../211502020.zip ../Code/*.c ../Code/*.h ../Code/makefile ../README.md ../report.pdf


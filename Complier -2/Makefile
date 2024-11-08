CC = gcc
CFLAGS = -Wall -g

all: parser run

# Link all the object files, including semantic.o and codegen.o
parser: parser.o lexer.o symbol_table.o ast.o semantic.o codegen.o
	$(CC) $(CFLAGS) -o parser parser.o lexer.o symbol_table.o ast.o semantic.o codegen.o -ll

# Generate parser.tab.c and parser.tab.h from parser.y and compile parser.o
parser.o: parser.y symbol_table.h ast.h semantic.h codegen.h
	bison -d parser.y
	$(CC) $(CFLAGS) -c parser.tab.c -o parser.o

# Generate lex.yy.c from lexer.l and compile lexer.o
lexer.o: lexer.l parser.tab.h ast.h codegen.h
	flex lexer.l
	$(CC) $(CFLAGS) -c lex.yy.c -o lexer.o

# Compile symbol_table.o from symbol_table.c
symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

# Compile ast.o from ast.c
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

# Compile semantic.o from semantic.c
semantic.o: semantic.c semantic.h ast.h symbol_table.h
	$(CC) $(CFLAGS) -c semantic.c

# Compile codegen.o from codegen.c
codegen.o: codegen.c codegen.h ast.h symbol_table.h
	$(CC) $(CFLAGS) -c codegen.c

# Run the parser with input.txt
run: parser
	./parser < input.txt

# Clean up generated files
clean:
	rm -f output.asm parser parser.o parser.tab.o lexer.o symbol_table.o ast.o semantic.o codegen.o parser.tab.c parser.tab.h lex.yy.c

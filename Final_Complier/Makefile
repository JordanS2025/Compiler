CC = gcc
CFLAGS = -Wall -g
LEX = flex
BISON = bison

# Default target builds and runs the parser
all: parser run

# Standard parser target
parser: parser.o lexer.o symbol_table.o ast.o semantic.o codegen.o mips.o
	$(CC) $(CFLAGS) -o parser parser.o lexer.o symbol_table.o ast.o semantic.o codegen.o mips.o

# Generate parser.tab.c and parser.tab.h
parser.o: parser.y symbol_table.h ast.h semantic.h codegen.h mips.h
	$(BISON) -d parser.y
	$(CC) $(CFLAGS) -c parser.tab.c -o parser.o

# Generate lex.yy.c and compile lexer.o
lexer.o: lexer.l parser.tab.h ast.h
	$(LEX) lexer.l
	$(CC) $(CFLAGS) -c lex.yy.c -o lexer.o

# Compile symbol_table.o
symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

# Compile ast.o
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

# Compile semantic.o
semantic.o: semantic.c semantic.h ast.h symbol_table.h
	$(CC) $(CFLAGS) -c semantic.c

# Compile codegen.o
codegen.o: codegen.c codegen.h ast.h symbol_table.h
	$(CC) $(CFLAGS) -c codegen.c

# Compile mips.o
mips.o: mips.c mips.h
	$(CC) $(CFLAGS) -c mips.c

# Run the parser with input
run: parser
	./parser < input.txt

# Clean up generated files
clean:
	rm -f parser parser.o lexer.o symbol_table.o ast.o semantic.o codegen.o mips.o parser.tab.c parser.tab.h lex.yy.c

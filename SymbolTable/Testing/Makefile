# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
BISON = bison
FLEX = flex
OUTPUT = my_compiler

# Files
LEXER = lexer.l
PARSER = parser.y
SYMBOLTABLE = symboltable.c symboltable.h
INPUT = input.txt
BISON_OUTPUT = parser.tab.c parser.tab.h
LEXER_OUTPUT = lex.yy.c

# Object files
OBJ = parser.tab.o lex.yy.o symboltable.o

# Default target: compile and run
all: $(OUTPUT)
	./$(OUTPUT) < $(INPUT)

# Generate the parser files
parser.tab.c parser.tab.h: $(PARSER)
	$(BISON) -d $(PARSER)

# Generate the lexer file
lex.yy.c: $(LEXER)
	$(FLEX) $(LEXER)

# Compile the parser
parser.tab.o: parser.tab.c parser.tab.h $(SYMBOLTABLE)
	$(CC) $(CFLAGS) -c parser.tab.c

# Compile the lexer
lex.yy.o: lex.yy.c parser.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c

# Compile the symbol table
symboltable.o: symboltable.c symboltable.h
	$(CC) $(CFLAGS) -c symboltable.c

# Link all object files to create the executable
$(OUTPUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJ) -ll

# Clean up generated files
clean:
	rm -f $(OUTPUT) $(LEXER_OUTPUT) $(BISON_OUTPUT) *.o


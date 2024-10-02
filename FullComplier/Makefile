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
AST = ast.c ast.h
SEMANTIC = semantic.c semantic.h    # Added semantic files
OPTIMIZER = optimizer.c optimizer.h # Added optimizer files
CODEGEN = codegen.c codegen.h       # Added code generator files
INPUT = input.txt

BISON_OUTPUT = parser.tab.c parser.tab.h
LEXER_OUTPUT = lex.yy.c

# Object files
OBJ = parser.tab.o \
      lex.yy.o \
      symboltable.o \
      ast.o \
      semantic.o \
      optimizer.o \
      codegen.o                    # Included codegen.o

# Phony targets
.PHONY: all run clean

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
parser.tab.o: parser.tab.c parser.tab.h $(SYMBOLTABLE) $(AST) $(SEMANTIC) $(CODEGEN)
	$(CC) $(CFLAGS) -c parser.tab.c

# Compile the lexer
lex.yy.o: lex.yy.c parser.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c

# Compile the symbol table
symboltable.o: symboltable.c symboltable.h
	$(CC) $(CFLAGS) -c symboltable.c

# Compile the AST
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

# Compile the Semantic Analyzer
semantic.o: semantic.c semantic.h $(AST) $(SYMBOLTABLE)
	$(CC) $(CFLAGS) -c semantic.c

# Compile the Optimizer
optimizer.o: optimizer.c optimizer.h
	$(CC) $(CFLAGS) -c optimizer.c

# Compile the Code Generator
codegen.o: codegen.c codegen.h $(SEMANTIC) $(AST) $(SYMBOLTABLE) $(OPTIMIZER)
	$(CC) $(CFLAGS) -c codegen.c

# Link all object files to create the executable
$(OUTPUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJ) -ll

# Clean up generated files
clean:
	rm -f $(OUTPUT) $(LEXER_OUTPUT) $(BISON_OUTPUT) *.o output.asm

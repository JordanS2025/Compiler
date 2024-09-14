%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"

void yyerror(const char *s);
extern int yylex();
extern int line_num;  // Correctly declared as extern

SymbolTable symtab; 
%}

%union {
    char* string;
    int number;
}

%token <string> ID
%token ASSIGNOP OP_ADD SEMICOLON WRITE
%token TYPE_INT TYPE_CHAR
%token <number> NUMBER

%left OP_ADD

%type <string> Type
%type <string> Expr

%%

Program:
    VarDeclList StmtList
    ;

VarDeclList:
    VarDeclList VarDecl
    | /* empty */
    ;

VarDecl:
    Type ID SEMICOLON
        {
            if (addSymbol(&symtab, $2, $1, line_num) != 0) {
                yyerror("Symbol already declared");
            }
            free($2); // Free the duplicated ID string
            free($1); // Free the duplicated Type string
            printSymbolTable(&symtab); // Print symbol table after declaration
        }
    | Type ID ASSIGNOP Expr SEMICOLON
        {
            if (addSymbol(&symtab, $2, $1, line_num) != 0) {
                yyerror("Symbol already declared");
            }
            // Handle initialization if necessary
            free($2); // Free the duplicated ID string
            free($1); // Free the duplicated Type string
            free($4); // Free the duplicated Expr string
            printSymbolTable(&symtab); // Print symbol table after declaration
        }
    ;

Type:
    TYPE_INT  { $$ = strdup("int"); }
    | TYPE_CHAR { $$ = strdup("char"); }
    ;

StmtList:
    StmtList Stmt
    | /* empty */
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON
        {
            SymbolTableEntry* entry = findSymbol(&symtab, $1);
            if (entry == NULL) {
                yyerror("Variable undeclared");
            }
            // Handle assignment if necessary
            free($1); // Free the duplicated ID string
            free($3); // Free the duplicated Expr string
            printSymbolTable(&symtab); // Print symbol table after statement
        }
    | WRITE ID SEMICOLON
        {
            SymbolTableEntry* entry = findSymbol(&symtab, $2);
            if (entry == NULL) {
                yyerror("Variable undeclared");
            }
            // Handle write operation if necessary
            free($2); // Free the duplicated ID string
            printSymbolTable(&symtab); // Print symbol table after statement
        }
    ;

Expr:
    ID OP_ADD ID
        { $$ = strdup("Expr(ID + ID)"); /* Placeholder */ }
    | ID OP_ADD NUMBER
        { $$ = strdup("Expr(ID + NUMBER)"); /* Placeholder */ }
    | NUMBER OP_ADD ID
        { $$ = strdup("Expr(NUMBER + ID)"); /* Placeholder */ }
    | NUMBER OP_ADD NUMBER
        { $$ = strdup("Expr(NUMBER + NUMBER)"); /* Placeholder */ }
    | ID
        { $$ = strdup("Expr(ID)"); /* Placeholder */ }
    | NUMBER
        { $$ = strdup("Expr(NUMBER)"); /* Placeholder */ }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s at line %d\n", s, line_num);
    freeSymbolTable(&symtab); // Clean up before exiting
    exit(EXIT_FAILURE);
}

int main() {
    printf("Enter your program:\n");
    initializeSymbolTable(&symtab);  // Initialize the symbol table

    if (yyparse() != 0) {
        fprintf(stderr, "Failed to parse the input.\n");
        freeSymbolTable(&symtab);
        return EXIT_FAILURE;
    }

    // Final symbol table print (optional)
    printSymbolTable(&symtab);
    freeSymbolTable(&symtab);
    return EXIT_SUCCESS;
}


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"
#include "ast.h" // Include AST header

void yyerror(const char *s);
extern int yylex();
extern int line_num;  

SymbolTable symtab; 
ASTNode* ast_root; // Global AST root
%}

%code requires {
    #include "ast.h" // Ensure ASTNode is defined in the header
}

%union {
    char* string;
    int number;
    ASTNode* node; // Add ASTNode* to the union
}

%token <string> ID
%token ASSIGNOP OP_ADD SEMICOLON WRITE
%token TYPE_INT TYPE_CHAR
%token <number> NUMBER

%left OP_ADD

%type <node> Program VarDeclList VarDecl StmtList Stmt Expr Type Term

%%

Program:
    VarDeclList StmtList
        {
            ast_root = createProgram($1, $2);
            $$ = ast_root;
        }
    ;

VarDeclList:
    VarDeclList VarDecl
        {
            $$ = createVarDeclList($1, $2);
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

VarDecl:
    Type ID SEMICOLON
        {
            if (addSymbol(&symtab, $2, $1->data.typeStr, line_num) != 0) {
                yyerror("Symbol already declared");
            }
            $$ = createVarDecl($1, $2, NULL);
            free($2); // Correct: Free duplicated ID string
            // Do NOT free $1->data.typeStr here
            printSymbolTable(&symtab); // Print symbol table after declaration
        }
    | Type ID ASSIGNOP Expr SEMICOLON
        {
            if (addSymbol(&symtab, $2, $1->data.typeStr, line_num) != 0) {
                yyerror("Symbol already declared");
            }
            $$ = createVarDecl($1, $2, $4);
            free($2); // Correct: Free duplicated ID string
            // Do NOT free $1->data.typeStr here
            // Do NOT free $4 here; it's owned by AST
            printSymbolTable(&symtab); // Print symbol table after declaration
        }
    ;

Type:
    TYPE_INT
        {
            $$ = createType("int");
        }
    | TYPE_CHAR
        {
            $$ = createType("char");
        }
    ;

StmtList:
    StmtList Stmt
        {
            $$ = createStmtList($1, $2);
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON
        {
            SymbolTableEntry* entry = findSymbol(&symtab, $1);
            if (entry == NULL) {
                yyerror("Variable undeclared");
            }
            $$ = createStmtAssign($1, $3);
            free($1); // Free duplicated ID string
            printSymbolTable(&symtab); // Print symbol table after statement
        }
    | WRITE ID SEMICOLON
        {
            SymbolTableEntry* entry = findSymbol(&symtab, $2);
            if (entry == NULL) {
                yyerror("Variable undeclared");
            }
            $$ = createStmtWrite($2);
            free($2); // Free duplicated ID string
            printSymbolTable(&symtab); // Print symbol table after statement
        }
    ;

Expr:
    Expr OP_ADD Term
        {
            $$ = createExprBinary("+", $1, $3);
        }
    | Term
        {
            $$ = $1;
        }
    ;

Term:
    ID
        {
            $$ = createExprId($1);
            free($1); // Free duplicated ID string
        }
    | NUMBER
        {
            $$ = createExprNumber($1);
        }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s at line %d\n", s, line_num);
    freeAST(ast_root); // Free the AST before exiting
    freeSymbolTable(&symtab); // Clean up before exiting
    exit(EXIT_FAILURE);
}

int main() {
    printf("Enter your program:\n");
    initializeSymbolTable(&symtab);  // Initialize the symbol table

    if (yyparse() == 0) {
        // Parsing succeeded
        printAST(ast_root, 0); // Print the AST
        freeAST(ast_root);      // Free the AST
    } else {
        fprintf(stderr, "Failed to parse the input.\n");
        freeSymbolTable(&symtab);
        return EXIT_FAILURE;
    }

    freeSymbolTable(&symtab);
    return EXIT_SUCCESS;
}


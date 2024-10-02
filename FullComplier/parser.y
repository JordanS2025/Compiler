%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"
#include "ast.h" 
#include "semantic.h"
#include "optimizer.h" // Include optimizer header
#include "codegen.h"    // Include code generator header

void yyerror(const char *s);
extern int yylex();
extern int line_num;  

SymbolTable symtab; 
ASTNode* ast_root; // Global AST root

// Declare tacList as external
extern TACList* tacList;
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
            // Print symbol table after declaration
            printSymbolTable(&symtab);
        }
    | Type ID ASSIGNOP Expr SEMICOLON
        {
            if (addSymbol(&symtab, $2, $1->data.typeStr, line_num) != 0) {
                yyerror("Symbol already declared");
            }
            $$ = createVarDecl($1, $2, $4);
            // Print symbol table after declaration
            printSymbolTable(&symtab);
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
            // Print symbol table after statement
            printSymbolTable(&symtab);
        }
    | WRITE ID SEMICOLON
        {
            SymbolTableEntry* entry = findSymbol(&symtab, $2);
            if (entry == NULL) {
                yyerror("Variable undeclared");
            }
            $$ = createStmtWrite($2);
            // Print symbol table after statement
            printSymbolTable(&symtab);
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
    printf("BEGINNING PROGRAM:\n");
    initializeSymbolTable(&symtab);  // Initialize the symbol table

    if (yyparse() == 0) {
        // Parsing succeeded
        printf("\nAbstract Syntax Tree (AST):\n");
        printAST(ast_root, 0); // Print the AST

        // Perform Semantic Analysis and Generate TAC
        initializeSemantic(&symtab);
        generateCode(ast_root, &symtab);

        printf("\nGenerated Three Address Code (TAC):\n");
        printTACList(tacList); // Print the generated TAC

        // Optimize the TAC
        initializeOptimizer();
        optimizeTAC();
        printf("\nOptimized Three Address Code (TAC):\n");
        printOptimizedTAC(tacList);
        finalizeOptimizer();

        // Initialize Code Generator
        if (!initializeCodeGenerator("output.asm")) {
            fprintf(stderr, "Failed to initialize code generator.\n");
            // Clean up resources before exiting
            freeTACList(tacList);
            freeAST(ast_root);
            freeSymbolTable(&symtab);
            return EXIT_FAILURE;
        }

        // Generate MIPS Code from Optimized TAC
        generateMIPS(tacList);

        // Finalize Code Generator
        finalizeCodeGenerator();

        // Inform the user
        printf("\nMIPS assembly code has been generated in 'output.asm'.\n");

        // Clean up resources
        freeTACList(tacList);     // Free the TAC list
        freeAST(ast_root);        // Free the AST
    } else {
        fprintf(stderr, "Failed to parse the input.\n");
        freeSymbolTable(&symtab);
        return EXIT_FAILURE;
    }

    freeSymbolTable(&symtab);
    return EXIT_SUCCESS;
}

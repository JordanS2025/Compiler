%{
#include <stdio.h>
#include <stdlib.h>
void yyerror(const char *s);
extern int yylex();
extern int line_num;  // Line number from lexer
%}

%token ID ASSIGNOP OP_ADD SEMICOLON WRITE NUMBER
%token TYPE_INT TYPE_CHAR
%left OP_ADD

%%

Program:
    VarDecl Block
    ;

VarDecl:
    Type ID SEMICOLON VarDecl
    |
    /* empty */
    ;

Type:
    TYPE_INT
    | TYPE_CHAR
    ;

Block:
    StmtList
    ;

StmtList:
    Stmt StmtList
    | /* empty */
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON
    | WRITE ID
    ;

Expr:
    ID OP_ADD ID
    | ID OP_ADD NUMBER
    | NUMBER OP_ADD ID
    | NUMBER OP_ADD NUMBER
    | ID
    | NUMBER
    ;

%%

void yyerror(const char *s) {
    printf("Parse error: %s at line %d\n", s, line_num);
    exit(1);
}

int main() {
    printf("Enter your program:\n");
    yyparse();
    return 0;
}

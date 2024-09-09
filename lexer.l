%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>

int line_num = 1; // Line number tracking

void yyerror(const char *s); // Error reporting
%}

%%

[0-9]+                {printf("%-15s  | %-15s\n", "TOKEN: NUMBER", yytext); return NUMBER; }
"int"                 { printf("%-15s | %-15s\n", "TOKEN: TYPE_INT", yytext); return TYPE_INT; }
"char"                { printf("%-15s | %-15s\n", "TOKEN: TYPE_CHAR", yytext); return TYPE_CHAR; }
[a-zA-Z_][a-zA-Z0-9_]* { printf("%-15s | %-15s\n", "TOKEN: ID", yytext); return ID; }
"="                   { printf("%-15s | %-15s\n", "TOKEN: ASSIGNOP", yytext); return ASSIGNOP; }
"+"                   { printf("%-15s | %-15s\n", "TOKEN: OP_ADD", yytext); return OP_ADD; }
"write"               { printf("%-15s | %-15s\n", "TOKEN: WRITE", yytext); return WRITE; }
";"                   { printf("%-15s | %-15s\n", "TOKEN: SEMICOLON", yytext); return SEMICOLON; }

[ \\t]+                { /* Ignore white spaces */ }
\n                    { line_num++; }
.                     { 
                        printf("Lexical error: Unrecognized character '%s' at line %d\\n", yytext, line_num); 
                        exit(1);
                      }

%%

int yywrap(void) {
    return 1;
}


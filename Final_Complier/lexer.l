/* lexer.l */

%{
#include "parser.tab.h"  // Correctly include the Bison-generated header
#include <string.h>
#include <stdio.h>
#include <stdlib.h> // For atof
int line_num = 1;  // Definition and initialization of line_num
%}

%%

[ \t\r]+           ; // Skip whitespace
\n                 { line_num++; } // Increment line number on newline

"int"              { 
                      printf("TOKEN: %-10s | %s\n", "TYPE_INT", yytext); 
                      return TYPE_INT; 
                  }

"char"             { 
                      printf("TOKEN: %-10s | %s\n", "TYPE_CHAR", yytext); 
                      return TYPE_CHAR; 
                  }

"float"            { 
                      printf("TOKEN: %-10s | %s\n", "TYPE_FLOAT", yytext); 
                      return TYPE_FLOAT; 
                  }

"array"            { 
                      printf("TOKEN: %-10s | %s\n", "ARRAY", yytext); 
                      return ARRAY; 
                  }

"return"           { 
                      printf("TOKEN: %-10s | %s\n", "RETURN", yytext); 
                      return RETURN; 
                  }

"main"             { 
                     printf("TOKEN: %-10s | %s\n", "MAIN", yytext); 
                      return MAIN; 
                  }
"while"             { 
                     printf("TOKEN: %-10s | %s\n", "WHILE", yytext); 
                      return WHILE; 
                  }
"="                { 
                     printf("TOKEN: %-10s | %s\n", "ASSIGNOP", yytext); 
                      return ASSIGNOP; 
                  }

"+"                { 
                      printf("TOKEN: %-10s | %s\n", "OP_ADD", yytext); 
                      return OP_ADD; 
                  }

"-"                { 
                     printf("TOKEN: %-10s | %s\n", "OP_SUB", yytext); 
                      return OP_SUB; 
                  }

"*"                { 
                     printf("TOKEN: %-10s | %s\n", "OP_MUL", yytext); 
                      return OP_MUL; 
                  }

"/"                { 
                      printf("TOKEN: %-10s | %s\n", "OP_DIV", yytext); 
                      return OP_DIV; 
                  }

";"                { 
                      printf("TOKEN: %-10s | %s\n", "SEMICOLON", yytext); 
                      return SEMICOLON; 
                  }

"write"            { 
                     printf("TOKEN: %-10s | %s\n", "WRITE", yytext); 
                      return WRITE; 
                  }
"if"               { 
                      printf("TOKEN: %-10s | %s\n", "IF", yytext); 
                      return IF; 
                  }

"else"             { 
                      printf("TOKEN: %-10s | %s\n", "ELSE", yytext); 
                      return ELSE; 
                  }

">"                { 
                      printf("TOKEN: %-10s | %s\n", "GT", yytext); 
                      return GT; 
                  }

"<"                { 
                      printf("TOKEN: %-10s | %s\n", "LT", yytext); 
                      return LT; 
                  }

">="               { 
                      printf("TOKEN: %-10s | %s\n", "GE", yytext); 
                      return GE; 
                  }

"<="               { 
                      printf("TOKEN: %-10s | %s\n", "LE", yytext); 
                      return LE; 
                  }

"=="               { 
                      printf("TOKEN: %-10s | %s\n", "EQ", yytext); 
                      return EQ; 
                  }

"!="               { 
                      printf("TOKEN: %-10s | %s\n", "NE", yytext); 
                      return NE; 
                  }

"!"                { 
                      printf("TOKEN: %-10s | %s\n", "NOT", yytext); 
                      return NOT; 
                  }

"&&"               { 
                      printf("TOKEN: %-10s | %s\n", "AND", yytext); 
                      return AND; 
                  }

"||"               { 
                      printf("TOKEN: %-10s | %s\n", "OR", yytext); 
                      return OR; 
                  }

"("                { 
                      printf("TOKEN: %-10s | %s\n", "LPAREN", yytext); 
                      return LPAREN; 
                  }

")"                { 
                      printf("TOKEN: %-10s | %s\n", "RPAREN", yytext); 
                      return RPAREN; 
                  }

"{"                { 
                      printf("TOKEN: %-10s | %s\n", "LBRACE", yytext); 
                      return LBRACE; 
                  }

"}"                { 
                      printf("TOKEN: %-10s | %s\n", "RBRACE", yytext); 
                      return RBRACE; 
                  }

"["                { 
                      printf("TOKEN: %-10s | %s\n", "LBRACKET", yytext); 
                      return LBRACKET; 
                  }

"]"                { 
                      printf("TOKEN: %-10s | %s\n", "RBRACKET", yytext); 
                      return RBRACKET; 
                  }

","                { 
                      printf("TOKEN: %-10s | %s\n", "COMMA", yytext); 
                      return COMMA; 
                  }

[0-9]+\.[0-9]+     { 
                     printf("TOKEN: %-10s | %s\n", "FLOAT_NUMBER", yytext); 
                      yylval.float_number = atof(yytext); 
                      return FLOAT_NUMBER; 
                  }

[0-9]+             { 
                      printf("TOKEN: %-10s | %s\n", "NUMBER", yytext); 
                      yylval.number = atoi(yytext); 
                      return NUMBER; 
                  }

[a-zA-Z_][a-zA-Z0-9_]* { 
                      printf("TOKEN: %-10s | %s\n", "ID", yytext); 
                      yylval.string = strdup(yytext); 
                      return ID; 
                  }

.                  { 
                      printf("Unrecognized character: %s at line %d\n", yytext, line_num); 
                      /* Handle errors or skip them here */ 
                  }

%%

int yywrap() {
    return 1;
}

/* parser.y */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "symbol_table.h"
#include "ast.h"
#include "semantic.h"  // Uncomment when 'traverse_ast' is implemented
#include "codegen.h"
#include "mips.h"

void compile(const char *filename);

extern int yylex();
extern int line_num;
extern char* yytext;

void yyerror(const char *s);

ASTNode* ast_root = NULL;
SymbolTable* sym_table;

%}

%define parse.error verbose
%defines

%code requires {
    #include "ast.h"
    #include "symbol_table.h"
}

%code top {
    #include "symbol_table.h"
}

%union {
    int number;
    float float_number;
    char* string;
    ASTNode* ast;
    Symbol* symbol;               
    ArraySizeNode* arr_sizes;
}

%token TYPE_INT TYPE_FLOAT TYPE_CHAR
%token ASSIGNOP OP_ADD OP_SUB OP_MUL OP_DIV
%token SEMICOLON
%token WRITE
%token ARRAY
%token RETURN
%token MAIN
%token IF ELSE
%token GT LT EQ NE NOT AND OR GE LE

%token <float_number> FLOAT_NUMBER
%token <number> NUMBER
%token <string> ID
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET COMMA
%token WHILE 

%type <ast> program function_definitions function_definition main_function function_body main_body declarations declaration statements statement 
%type <ast> parameters
%type <ast> parameter
%type <ast> write_stmt assignment expression array_init initializer_list return_statement main_return_statement
%type <ast> argument_list arguments
%type <ast> if_stmt else_part while_stmt
%type <arr_sizes> array_sizes

%left OR
%left AND
%left EQ NE
%left GE LE GT LT
%left OP_ADD OP_SUB
%left OP_MUL OP_DIV
%right NOT

%start program

%%

/* Grammar Rules with Semantic Actions */

program
    : function_definitions main_function
        {
            /* Create the root AST node */
            $$ = create_ast_node(AST_PROGRAM);
            if ($1) add_child($$, $1);    /* function_definitions */
            if ($2) add_child($$, $2);    /* main_function */
            ast_root = $$;
        }
    ;

main_function
    : TYPE_INT MAIN LPAREN RPAREN LBRACE main_body RBRACE
        {
            /* Create the main function AST node */
            $$ = create_ast_node(AST_MAIN_FUNCTION);

            /* Add 'main' to the symbol table */
            add_symbol("main", DT_INT, SYMBOL_FUNCTION, DT_INT, NULL, NULL, 0);

            /* Enter new scope for main function body */
            enter_scope();

            /* Attach main body */
            if ($6) add_child($$, $6);    /* main_body */

            /* Exit scope after main function body */
            exit_scope();
        }
    ;


function_definitions
    : function_definitions function_definition
        {
            /* Link the function_definition to the list */
            if ($1) {
                ASTNode* temp = $1;
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = $2;
                $$ = $1;
            } else {
                $$ = $2;
            }
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;


function_definition
    : TYPE_INT ID LPAREN parameters RPAREN LBRACE function_body RBRACE
        {
            /* Create a function definition AST node */
            $$ = create_ast_node(AST_FUNCTION_DEFINITION);
            $$->name = strdup($2);
            $$->data_type = DT_INT;

            /* Add function to symbol table */
            add_symbol($2, DT_INT, SYMBOL_FUNCTION, DT_INT, NULL, NULL, 0);

            /* Enter new scope for function body */
            enter_scope();

            /* Attach parameters and body */
            if ($4) add_child($$, $4);    /* parameters */
            if ($7) add_child($$, $7);    /* function_body */

            /* Exit scope after function body */
            exit_scope();
        }
    ;



parameters
    : parameters COMMA parameter
        {
            /* Link parameters */
            if ($1) {
                ASTNode* temp = $1;
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = $3;
                $$ = $1;
            } else {
                $$ = $3;
            }
        }
    | parameter
        {
            /* Single parameter */
            $$ = $1;
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;


parameter
    : TYPE_INT ID
        {
            /* Create a parameter declaration AST node */
            ASTNode* param_node = create_ast_node(AST_DECLARATION);
            param_node->name = strdup($2);
            param_node->data_type = DT_INT;
            param_node->category = SYMBOL_VARIABLE;

            /* Add to symbol table */
            add_symbol($2, DT_INT, SYMBOL_VARIABLE, DT_VOID, NULL, NULL, 0);

            $$ = param_node;
        }
    | TYPE_FLOAT ID
        {
            /* Create a parameter declaration AST node */
            ASTNode* param_node = create_ast_node(AST_DECLARATION);
            param_node->name = strdup($2);
            param_node->data_type = DT_FLOAT;
            param_node->category = SYMBOL_VARIABLE;

            /* Add to symbol table */
            add_symbol($2, DT_FLOAT, SYMBOL_VARIABLE, DT_VOID, NULL, NULL, 0);

            $$ = param_node;
        }
    | TYPE_CHAR ID
        {
            /* Create a parameter declaration AST node */
            ASTNode* param_node = create_ast_node(AST_DECLARATION);
            param_node->name = strdup($2);
            param_node->data_type = DT_CHAR;
            param_node->category = SYMBOL_VARIABLE;

            /* Add to symbol table */
            add_symbol($2, DT_CHAR, SYMBOL_VARIABLE, DT_VOID, NULL, NULL, 0);

            $$ = param_node;
        }
    ;


function_body
    : declarations statements return_statement
        {
            /* Create a block AST node */
            $$ = create_ast_node(AST_BLOCK);
            if ($1) add_child($$, $1);    /* declarations */
            if ($2) add_child($$, $2);    /* statements */
            if ($3) add_child($$, $3);    /* return_statement */
        }
    ;


main_body
    : declarations statements main_return_statement
        {
            /* Create a block AST node */
            $$ = create_ast_node(AST_BLOCK);
            if ($1) add_child($$, $1);    /* declarations */
            if ($2) add_child($$, $2);    /* statements */
            if ($3) add_child($$, $3);    /* main_return_statement */
        }
    ;


return_statement
    : RETURN expression SEMICOLON
        {
            /* Create a return statement AST node */
            $$ = create_ast_node(AST_RETURN);
            if ($2) add_child($$, $2);    /* expression */
        }
    ;


main_return_statement
    : RETURN NUMBER SEMICOLON
        {
            /* Create a return statement AST node with integer literal */
            $$ = create_ast_node(AST_RETURN);
            ASTNode* num_node = create_ast_node(AST_EXPRESSION);
            num_node->operator = strdup("NUMBER");
            num_node->value = $2;
            add_child($$, num_node);
        }
    ;


declarations
    : declarations declaration
        {
            /* Link declarations */
            if ($1) {
                ASTNode* temp = $1;
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = $2;
                $$ = $1;
            } else {
                $$ = $2;
            }
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;


declaration
    : ARRAY TYPE_INT ID array_sizes ASSIGNOP array_init SEMICOLON
        {
            /* Create a declaration node for array */
            $$ = create_ast_node(AST_DECLARATION);
            $$->name = strdup($3);
            $$->data_type = DT_INT;
            $$->category = SYMBOL_ARRAY;
            $$->array_sizes = $4->sizes;
            $$->dimensions = $4->dimensions;

            /* Add array to symbol table */
            add_symbol($3, DT_ARRAY, SYMBOL_ARRAY, DT_VOID, NULL, $4->sizes, $4->dimensions);

            /* Attach array initialization */
            if ($6) add_child($$, $6);
        }
    | TYPE_INT ID ASSIGNOP expression SEMICOLON
        {
            /* Create a declaration node for int */
            $$ = create_ast_node(AST_DECLARATION);
            $$->name = strdup($2);
            $$->data_type = DT_INT;
            $$->category = SYMBOL_VARIABLE;

            /* Add variable to symbol table */
            add_symbol($2, DT_INT, SYMBOL_VARIABLE, DT_VOID, NULL, NULL, 0);

            /* Attach initialization expression */
            if ($4) add_child($$, $4);
        }
    | TYPE_FLOAT ID ASSIGNOP expression SEMICOLON
        {
            /* Create a declaration node for float */
            $$ = create_ast_node(AST_DECLARATION);
            $$->name = strdup($2);
            $$->data_type = DT_FLOAT;
            $$->category = SYMBOL_VARIABLE;

            /* Add variable to symbol table */
            add_symbol($2, DT_FLOAT, SYMBOL_VARIABLE, DT_VOID, NULL, NULL, 0);

            /* Attach initialization expression */
            if ($4) add_child($$, $4);
        }
    | TYPE_CHAR ID ASSIGNOP expression SEMICOLON
        {
            /* Create a declaration node for char */
            $$ = create_ast_node(AST_DECLARATION);
            $$->name = strdup($2);
            $$->data_type = DT_CHAR;
            $$->category = SYMBOL_VARIABLE;

            /* Add variable to symbol table */
            add_symbol($2, DT_CHAR, SYMBOL_VARIABLE, DT_VOID, NULL, NULL, 0);

            /* Attach initialization expression */
            if ($4) add_child($$, $4);
        }
    ;


array_sizes
    : LBRACKET NUMBER RBRACKET
        {
            /* Initialize array sizes */
            ArraySizeNode* node = (ArraySizeNode*)malloc(sizeof(ArraySizeNode));
            node->sizes = (int*)malloc(sizeof(int));
            node->sizes[0] = $2;
            node->dimensions = 1;
            $$ = node;
        }
    | array_sizes LBRACKET NUMBER RBRACKET
        {
            /* Extend array sizes */
            ArraySizeNode* node = $1;
            node->sizes = (int*)realloc(node->sizes, sizeof(int) * (node->dimensions + 1));
            node->sizes[node->dimensions] = $3;
            node->dimensions += 1;
            $$ = node;
        }
    ;


array_init
    : LBRACE initializer_list RBRACE
        {
            /* Create an array initialization AST node */
            $$ = create_ast_node(AST_ARRAY_INIT);
            if ($2) add_child($$, $2);
        }
    ;


initializer_list
    : initializer_list COMMA expression
        {
            /* Link initializers */
            if ($1) {
                ASTNode* temp = $1;
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = $3;
                $$ = $1;
            } else {
                $$ = $3;
            }
        }
    | expression
        {
            /* Single initializer */
            $$ = $1;
        }
    ;


statements
    : statements statement
        {
            /* Link statements */
            if ($1) {
                ASTNode* temp = $1;
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = $2;
                $$ = $1;
            } else {
                $$ = $2;
            }
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;


statement
    : declaration
        {
            /* Statement is a declaration */
            $$ = $1;
        }
    | assignment
        {
            /* Statement is an assignment */
            $$ = $1;
        }
    | write_stmt
        {
            /* Statement is a write operation */
            $$ = $1;
        }
    | if_stmt
        {
            /* Statement is an if statement */
            $$ = $1;
        }
    | while_stmt
        {
            /* Statement is a while loop */
            $$ = $1;
        }
    | expression SEMICOLON
        {
            /* Expression as a statement */
            $$ = $1;
        }
    ;


if_stmt
    : IF LPAREN expression RPAREN LBRACE statements RBRACE else_part
        {
            /* Create an if statement AST node */
            $$ = create_ast_node(AST_IF);

            /* Attach condition and then body */
            if ($3) $$->condition = $3;
            if ($6) $$->body = $6;

            /* Attach else part */
            if ($8) {
                add_child($$, $8);    /* else_part */
            }
        }
    ;


else_part
    : ELSE LBRACE statements RBRACE
        {
            /* Create an else part AST node */
            ASTNode* else_node = create_ast_node(AST_IF);    /* Reusing AST_IF type for else */

            /* Enter new scope for else block */
            enter_scope();

            /* Attach else body */
            if ($3) else_node->body = $3;

            /* Exit scope after else block */
            exit_scope();

            $$ = else_node;
        }
    | ELSE if_stmt
        {
            /* Handle 'else if' by linking to another if statement */
            $$ = $2;
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;


while_stmt
    : WHILE LPAREN expression RPAREN LBRACE statements RBRACE
        {
            /* Create a while statement AST node */
            $$ = create_ast_node(AST_WHILE);

            /* Attach condition and body */
            if ($3) $$->condition = $3;
            if ($6) $$->body = $6;
        }
    ;


assignment
    : ID ASSIGNOP expression SEMICOLON
        {
            /* Create an assignment AST node */
            $$ = create_ast_node(AST_ASSIGNMENT);
            $$->name = strdup($1);
            if ($3) add_child($$, $3);    /* expression */
        }
    | ID LBRACKET expression RBRACKET ASSIGNOP expression SEMICOLON
        {
            /* Create an array assignment AST node */
            $$ = create_ast_node(AST_ASSIGNMENT);
            $$->name = strdup($1);

            /* Create an array access node */
            ASTNode* array_access = create_ast_node(AST_ARRAY_ACCESS);
            array_access->string = strdup($1);
            if ($3) add_child(array_access, $3);  /* index expression */

            /* Attach array access and value expression */
            if ($6) add_child($$, $6);         /* value expression */
            add_child($$, array_access);       /* Attach array access */
        }
    ;


write_stmt
    : WRITE ID SEMICOLON
        {
            /* Create a write statement AST node */
            Symbol* sym = lookup_symbol($2);
            if (!sym) {
                yyerror("Undeclared variable in write statement.");
                $$ = NULL;
            } else {
                $$ = create_ast_node(AST_WRITE);
                $$->name = strdup($2);
            }
        }
    | WRITE ID LBRACKET expression RBRACKET SEMICOLON
        {
            /* Handle writing array element */
            Symbol* sym = lookup_symbol($2);
            if (!sym || sym->category != SYMBOL_ARRAY) {
                yyerror("Undeclared array or wrong category in write statement.");
                $$ = NULL;
            } else {
                $$ = create_ast_node(AST_WRITE);
                $$->name = strdup($2);

                /* Create an array access node */
                ASTNode* array_access = create_ast_node(AST_ARRAY_ACCESS);
                array_access->string = strdup($2);
                if ($4) add_child(array_access, $4); /* index expression */

                add_child($$, array_access);
            }
        }
    ;


expression
    : expression OP_ADD expression
        {
            /* Create an addition expression node */
            $$ = create_expression_node("+", $1, $3);
        }
    | expression OP_SUB expression
        {
            /* Create a subtraction expression node */
            $$ = create_expression_node("-", $1, $3);
        }
    | expression OP_MUL expression
        {
            /* Create a multiplication expression node */
            $$ = create_expression_node("*", $1, $3);
        }
    | expression OP_DIV expression
        {
            /* Create a division expression node */
            $$ = create_expression_node("/", $1, $3);
        }
    | expression OR expression
        {
            /* Create a logical OR expression node */
            $$ = create_expression_node("||", $1, $3);
        }
    | expression AND expression
        {
            /* Create a logical AND expression node */
            $$ = create_expression_node("&&", $1, $3);
        }
    | expression EQ expression
        {
            /* Create an equality expression node */
            $$ = create_expression_node("==", $1, $3);
        }
    | expression NE expression
        {
            /* Create an inequality expression node */
            $$ = create_expression_node("!=", $1, $3);
        }
    | expression GE expression
        {
            /* Create a greater or equal expression node */
            $$ = create_expression_node(">=", $1, $3);
        }
    | expression LE expression
        {
            /* Create a less or equal expression node */
            $$ = create_expression_node("<=", $1, $3);
        }
    | expression GT expression
        {
            /* Create a greater than expression node */
            $$ = create_expression_node(">", $1, $3);
        }
    | expression LT expression
        {
            /* Create a less than expression node */
            $$ = create_expression_node("<", $1, $3);
        }
    | NOT expression
        {
            /* Create a logical NOT expression node */
            $$ = create_expression_node("!", $2, NULL);
        }
    | ID LPAREN argument_list RPAREN
        {
            /* Handle function call */
            Symbol* sym = lookup_symbol($1);
            if (!sym || sym->category != SYMBOL_FUNCTION) {
                yyerror("Undeclared function.");
                $$ = NULL;
            } else {
                $$ = create_ast_node(AST_FUNCTION_CALL);
                $$->string = strdup($1);    /* Function name */
                if ($3) $$->arguments = $3; /* Arguments */
            }
        }
    | ID LBRACKET expression RBRACKET
        {
            /* Handle array access */
            Symbol* sym = lookup_symbol($1);
            if (!sym || sym->category != SYMBOL_ARRAY) {
                yyerror("Undeclared array or wrong category in expression.");
                $$ = NULL;
            } else {
                $$ = create_ast_node(AST_ARRAY_ACCESS);
                $$->string = strdup($1);    /* Array name */
                if ($3) add_child($$, $3);   /* Index expression */
            }
        }
    | ID
        {
            /* Handle variable */
            Symbol* sym = lookup_symbol($1);
            if (!sym) {
                yyerror("Undeclared variable in expression.");
                $$ = NULL;
            } else {
                $$ = create_ast_node(AST_EXPRESSION);
                $$->operator = strdup("ID");
                $$->string = strdup($1);
            }
        }
    | NUMBER
        {
            /* Handle integer literal */
            $$ = create_ast_node(AST_EXPRESSION);
            $$->operator = strdup("NUMBER");
            $$->value = $1;
        }
    | FLOAT_NUMBER
        {
            /* Handle float literal */
            $$ = create_ast_node(AST_EXPRESSION);
            $$->operator = strdup("FLOAT_NUMBER");
            $$->float_value = $1;
        }
    | LPAREN expression RPAREN
        {
            /* Parenthesized expression */
            $$ = $2;
        }
    ;


argument_list
    : arguments
        {
            $$ = $1; // Pass the arguments list
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;


arguments
    : arguments COMMA expression
        {
            /* Link arguments */
            if ($1) {
                ASTNode* temp = $1;
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = $3;
                $$ = $1;
            } else {
                $$ = $3;
            }
        }
    | expression
        {
            /* Single argument */
            $$ = $1;
        }
    ;

%%

/* C Code Section */

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s. Token: '%s'\n", line_num, s, yytext);
}

void compile(const char *filename) {
    printf("Compiling file: %s\n", filename);
    
    /* Initialize the symbol table */
    init_symbol_table();

    /* Start parsing */
    if (yyparse() == 0) {
        printf("Parsing completed successfully.\n");

        /* Perform semantic analysis */
        traverse_ast(ast_root);
        printf("Semantic analysis completed successfully.\n");

        /* Printing the Symbol Table */
        print_symbol_table();

        /* Print the AST for debugging */
        print_ast(ast_root, 0);

        /* Generate TAC */
        printf("Generating TAC...\n");
        generate_tac(ast_root);
        printf("TAC generation completed.\n");

        /* Generate MIPS assembly directly from AST */
        printf("Generating MIPS assembly...\n");
        generate_mips(ast_root);
        printf("MIPS assembly generation completed.\n");

        /* Free resources */
        free_all_symbol_tables();
        free_ast(ast_root);
    } else {
        fprintf(stderr, "Parsing failed. Please check your input.\n");
    }
}
int main(int argc, char** argv) {
    /* Record start time */
    clock_t start_time = clock();

    /* Initialize the symbol table */
    init_symbol_table();

    /* Start parsing */
    if (yyparse() == 0) {
        printf("Parsing completed successfully.\n");

        printf("About to print symbol table...\n");
        print_symbol_table();
        printf("Symbol table printing completed.\n");

        /* Perform semantic analysis */
        traverse_ast(ast_root);
        printf("Semantic analysis completed successfully.\n");


        /* Print the AST for debugging */
        print_ast(ast_root, 0);

        /* Generate TAC */
        printf("Generating TAC...\n");
        generate_tac(ast_root);
        printf("TAC generation completed.\n");

        /* Generate MIPS assembly directly from AST */
        printf("Generating MIPS assembly...\n");
        generate_mips(ast_root);
        printf("MIPS assembly generation completed.\n");

        /* Free resources */
        free_all_symbol_tables();
        free_ast(ast_root);
    } else {
        fprintf(stderr, "Parsing failed. Please check your input.\n");
    }

    /* Record end time */
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Compilation time: %.4f seconds\n", elapsed_time);

    return 0;
}

/* parser.y */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "ast.h"
#include "semantic.h"
#include "codegen.h"

extern int yylex();
extern int line_num;
extern char* yytext;

void yyerror(const char *s);

ASTNode* ast_root;
SymbolTable* sym_table;
%}

%code requires {
    #include "ast.h" // Ensure ASTNode is defined in the header
}

%union {
    int number;
    float float_number;
    char* string;
    ASTNode* ast;
}

%token TYPE_INT TYPE_FLOAT TYPE_CHAR
%token ASSIGNOP OP_ADD OP_SUB OP_MUL OP_DIV
%token SEMICOLON
%token WRITE
%token ARRAY
%token RETURN
%token <float_number> FLOAT_NUMBER
%token <number> NUMBER
%token <string> ID
%token '[' ']' ',' '{' '}' '(' ')'

%type <ast> program function_definitions function_definition main_function function_body main_body declarations declaration statements statement 
%type <ast> write_stmt assignment expression array_sizes array_init initializer_list return_statement main_return_statement

/* Define operator precedence and associativity */
%left OP_ADD OP_SUB
%left OP_MUL OP_DIV

%%

program
    : function_definitions main_function
        {
            ast_root = create_program_node($1, $2);
        }
    ;

function_definitions
    : function_definitions function_definition
        {
            $$ = add_to_function_list($1, $2);
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

function_definition
    : TYPE_INT ID '(' ')' '{' function_body '}'
        {
            if (!add_function_symbol(sym_table, $2, "int")) {
                fprintf(stderr, "Error: Function '%s' already declared at line %d\n", $2, line_num);
                exit(1);
            }
            $$ = create_function_def_node("int", $2, $6);
        }
    ;

main_function
    : TYPE_INT ID '(' ')' '{' main_body '}'
        {
            if (strcmp($2, "main") != 0) {
                fprintf(stderr, "Error: Main function must be named 'main' at line %d\n", line_num);
                exit(1);
            }
            $$ = create_main_function_node($6);
        }
    ;

function_body
    : declarations statements return_statement
        {
            $$ = create_function_body_node($1, $2, $3);
        }
    ;

main_body
    : declarations statements main_return_statement
        {
            $$ = create_main_body_node($1, $2, $3);
        }
    ;

return_statement
    : RETURN expression SEMICOLON
        {
            $$ = create_return_node($2);
        }
    ;

main_return_statement
    : RETURN NUMBER SEMICOLON
        {
            if ($2 != 0) {
                fprintf(stderr, "Error: Main function must return 0 at line %d\n", line_num);
                exit(1);
            }
            $$ = create_return_node(create_number_node($2));
        }
    ;

declarations
    : declarations declaration
        {
            $$ = add_to_var_decl_list($1, $2);
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

declaration
    : ARRAY TYPE_INT ID array_sizes ASSIGNOP array_init SEMICOLON
    {
        if (!add_array_symbol(sym_table, $3, "int", $4->num_dimensions, $4->sizes)) {
            fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $3, line_num);
            exit(1);
        }
        // Set initialized to true since it's initialized upon declaration
        Symbol* sym = get_symbol(sym_table, $3);
        sym->initialized = true;

        // Correct: Pass both array_name ($3) and initializer_list ($6)
        ASTNode* array_init_node = create_array_init_node($3, $6); // Pass array_name and initializer_list
        ASTNode* initializer = create_initial_expression_node(array_init_node); // Wrap in "Initializer" node
        ASTNode* decl = create_array_decl_node("int", $3, $4, initializer);
        $$ = decl;
    }

    | ARRAY TYPE_FLOAT ID array_sizes ASSIGNOP array_init SEMICOLON
        {
            if (!add_array_symbol(sym_table, $3, "float", $4->num_dimensions, $4->sizes)) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $3, line_num);
                exit(1);
            }
            // Set initialized to true
            Symbol* sym = get_symbol(sym_table, $3);
            sym->initialized = true;

            // Correct: Pass both array_name ($3) and initializer_list ($6)
            ASTNode* array_init_node = create_array_init_node($3, $6); // Pass array_name and initializer_list
            ASTNode* initializer = create_initial_expression_node(array_init_node); // Wrap in "Initializer" node
            ASTNode* decl = create_array_decl_node("float", $3, $4, initializer);
            $$ = decl;
        }

    | ARRAY TYPE_CHAR ID array_sizes ASSIGNOP array_init SEMICOLON
        {
            if (!add_array_symbol(sym_table, $3, "char", $4->num_dimensions, $4->sizes)) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $3, line_num);
                exit(1);
            }
            // Set initialized to true
            Symbol* sym = get_symbol(sym_table, $3);
            sym->initialized = true;

            // Correct: Pass both array_name ($3) and initializer_list ($6)
            ASTNode* array_init_node = create_array_init_node($3, $6); // Pass array_name and initializer_list
            ASTNode* initializer = create_initial_expression_node(array_init_node); // Wrap in "Initializer" node
            ASTNode* decl = create_array_decl_node("char", $3, $4, initializer);
            $$ = decl;
        }
   | TYPE_INT ID ASSIGNOP expression SEMICOLON
    {
        if (!add_symbol(sym_table, $2, "int")) {
            fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $2, line_num);
            exit(1);
        }
        // Set initialized to true
        Symbol* sym = get_symbol(sym_table, $2);
        sym->initialized = true;

        ASTNode* initializer = create_initial_expression_node($4); // Wrap expression in "Initializer" node
        ASTNode* decl = create_var_decl_node("int", $2, NULL, initializer);
        $$ = decl;
    }

    | TYPE_INT ID SEMICOLON
        {
            if (!add_symbol(sym_table, $2, "int")) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $2, line_num);
                exit(1);
            }
            // Variable declared without initialization - Not allowed
            fprintf(stderr, "Error: Variable '%s' must be initialized at declaration at line %d\n", $2, line_num);
            exit(1);
        }
    | TYPE_FLOAT ID ASSIGNOP expression SEMICOLON
        {
            if (!add_symbol(sym_table, $2, "float")) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $2, line_num);
                exit(1);
            }
            // Set initialized to true
            Symbol* sym = get_symbol(sym_table, $2);
            sym->initialized = true;

            ASTNode* decl = create_var_decl_node("float", $2, NULL, $4);
            $$ = decl;
        }
    | TYPE_FLOAT ID SEMICOLON
        {
            if (!add_symbol(sym_table, $2, "float")) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $2, line_num);
                exit(1);
            }
            // Variable declared without initialization - Not allowed
            fprintf(stderr, "Error: Variable '%s' must be initialized at declaration at line %d\n", $2, line_num);
            exit(1);
        }
    | TYPE_CHAR ID ASSIGNOP expression SEMICOLON
        {
            if (!add_symbol(sym_table, $2, "char")) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $2, line_num);
                exit(1);
            }
            // Set initialized to true
            Symbol* sym = get_symbol(sym_table, $2);
            sym->initialized = true;

            ASTNode* decl = create_var_decl_node("char", $2, NULL, $4);
            $$ = decl;
        }
    | TYPE_CHAR ID SEMICOLON
        {
            if (!add_symbol(sym_table, $2, "char")) {
                fprintf(stderr, "Error: Variable '%s' already declared at line %d\n", $2, line_num);
                exit(1);
            }
            // Variable declared without initialization - Not allowed
            fprintf(stderr, "Error: Variable '%s' must be initialized at declaration at line %d\n", $2, line_num);
            exit(1);
        }
    ;

array_sizes
    : array_sizes '[' NUMBER ']'
        { 
            ASTNode* size_node = create_number_node($3);
            $$ = add_to_array_sizes_list($1, size_node);
        }
    | '[' NUMBER ']'
        { 
            ASTNode* size_node = create_number_node($2);
            size_node->num_dimensions = 1; // Initialize number of dimensions
            size_node->sizes[0] = $2;
            $$ = size_node;
        }
    ;

statements
    : statements statement
        {
            $$ = add_to_stmt_list($1, $2);
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

statement
    : assignment
        { $$ = $1; }
    | write_stmt
        { $$ = $1; }
    | expression SEMICOLON
        { $$ = $1; } // For function calls as statements
    ;

assignment
    : ID ASSIGNOP expression SEMICOLON
        {
            // Check if the expression is a function call
            if (strcmp($3->node_type, "FunctionCall") == 0) {
                fprintf(stderr, "Error: Cannot assign function call result to variable '%s' at line %d\n", $1, line_num);
                exit(1);
            }

            Symbol* sym = get_symbol(sym_table, $1);
            if (!sym) {
                fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $1, line_num);
                exit(1);
            }
            if (sym->is_array) {
                fprintf(stderr, "Error: Cannot assign to array '%s' without indices at line %d\n", $1, line_num);
                exit(1);
            }
            // Variable is now initialized
            sym->initialized = true;

            ASTNode* assign = create_assignment_node($1, $3);
            $$ = assign;
        }
    | ID '[' expression ']' ASSIGNOP expression SEMICOLON
        {
            Symbol* sym = get_symbol(sym_table, $1);
            if (!sym) {
                fprintf(stderr, "Error: Undeclared array '%s' at line %d\n", $1, line_num);
                exit(1);
            }
            if (!sym->is_array) {
                fprintf(stderr, "Error: Identifier '%s' is not an array at line %d\n", $1, line_num);
                exit(1);
            }
            // Since we're assigning to an element, we can consider the array initialized
            sym->initialized = true;

            ASTNode* assign = create_array_assignment_node($1, $3, $6);
            $$ = assign;
        }
    ;

write_stmt
    : WRITE ID SEMICOLON
        {
            Symbol* sym = get_symbol(sym_table, $2);
            if (!sym) {
                fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $2, line_num);
                exit(1);
            }
            if (sym->is_array) {
                fprintf(stderr, "Error: Cannot write entire array '%s' without indices at line %d\n", $2, line_num);
                exit(1);
            }
            if (!sym->initialized) {
                fprintf(stderr, "Error: Variable '%s' used before initialization at line %d\n", $2, line_num);
                exit(1);
            }

            ASTNode* write = create_write_node($2);
            $$ = write;
        }
    | WRITE ID '[' expression ']' SEMICOLON
        {
            Symbol* sym = get_symbol(sym_table, $2);
            if (!sym) {
                fprintf(stderr, "Error: Undeclared array '%s' at line %d\n", $2, line_num);
                exit(1);
            }
            if (!sym->is_array) {
                fprintf(stderr, "Error: Identifier '%s' is not an array at line %d\n", $2, line_num);
                exit(1);
            }
            if (!sym->initialized) {
                fprintf(stderr, "Error: Array '%s' used before initialization at line %d\n", $2, line_num);
                exit(1);
            }

            ASTNode* write = create_write_node_with_access($2, $4);
            $$ = write;
        }
    ;

expression
    : expression OP_ADD expression
        { $$ = create_binary_op_node("+", $1, $3); }
    | expression OP_SUB expression
        { $$ = create_binary_op_node("-", $1, $3); }
    | expression OP_MUL expression
        { $$ = create_binary_op_node("*", $1, $3); }
    | expression OP_DIV expression
        { $$ = create_binary_op_node("/", $1, $3); }
    | ID '[' expression ']'
        {
            Symbol* sym = get_symbol(sym_table, $1);
            if (!sym) {
                fprintf(stderr, "Error: Undeclared identifier '%s' at line %d\n", $1, line_num);
                exit(1);
            }
            if (!sym->is_array) {
                fprintf(stderr, "Error: Identifier '%s' is not an array at line %d\n", $1, line_num);
                exit(1);
            }
            if (!sym->initialized) {
                fprintf(stderr, "Error: Array '%s' used before initialization at line %d\n", $1, line_num);
                exit(1);
            }

            $$ = create_array_access_node($1, $3);
        }
    | ID
        {
            Symbol* sym = get_symbol(sym_table, $1);
            if (!sym) {
                fprintf(stderr, "Error: Undeclared identifier '%s' at line %d\n", $1, line_num);
                exit(1);
            }
            if (sym->is_array) {
                fprintf(stderr, "Error: Cannot use array '%s' without indices at line %d\n", $1, line_num);
                exit(1);
            }
            if (!sym->initialized) {
                fprintf(stderr, "Error: Variable '%s' used before initialization at line %d\n", $1, line_num);
                exit(1);
            }

            $$ = create_id_node($1);
        }
    | NUMBER
        { $$ = create_number_node($1); }
    | FLOAT_NUMBER
        { $$ = create_float_node($1); }
    | '(' expression ')'
        { $$ = $2; }
    | ID '(' ')'
        {
            if (!function_exists(sym_table, $1)) {
                fprintf(stderr, "Error: '%s' is not a function at line %d\n", $1, line_num);
                exit(1);
            }
            $$ = create_function_call_node($1);
        }
    ;

array_init
    : '{' initializer_list '}'
        { $$ = $2; } // Return the initializer_list directly
    ;

initializer_list
    : initializer_list ',' expression
        { $$ = add_to_initializer_list($1, $3); }
    | expression
        { $$ = create_initial_expression_node($1); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", line_num, s);
}

int main(int argc, char** argv) {
    sym_table = create_symbol_table();
    if (yyparse() == 0) {
        printf("Parsing completed successfully.\n");
        print_symbol_table(sym_table);

        // Perform semantic analysis
        perform_semantic_analysis(ast_root, sym_table);
        printf("Semantic analysis completed successfully.\n");
        
        printf("Abstract Syntax Tree (AST):\n");
        traverse_ast(ast_root, 0);  // Pass initial indentation level

        // Generate Three-Address Code (TAC) and Assembly
        printf("Generating Three-Address Code (TAC) and Assembly...\n");
        generate_TAC(ast_root, sym_table);
        printf("TAC and Assembly generation completed successfully.\n");
        printf("Assembly code written to output.asm\n");
    }
    else {
        fprintf(stderr, "Parsing failed due to syntax errors.\n");
    }
    free_symbol_table(sym_table);
    free_ast(ast_root);
    return 0;
}


/* ast.h */

#ifndef AST_H
#define AST_H

#include "symbol_table.h"

/* Enumeration of all possible AST node types */
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION_DEFINITION,
    AST_MAIN_FUNCTION,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_WRITE,
    AST_IF,
    AST_WHILE,
    AST_RETURN,
    AST_EXPRESSION,
    AST_BLOCK,
    AST_ARRAY_INIT,
    AST_FUNCTION_CALL,   // Added this line
    AST_ARRAY_ACCESS,    // Added this line
    /* Add more node types as needed */
} ASTNodeType;

/* Forward declaration for ASTNode */
typedef struct ASTNode ASTNode;

/* Structure for AST Nodes */
struct ASTNode {
    ASTNodeType type;        /* Type of the AST node */
    char* name;              /* Name associated with the node (e.g., variable name) */
    DataType data_type;      /* Data type (int, float, etc.) */
    SymbolCategory category; /* Category (Variable, Function, etc.) */
    int increment;
    /* For expressions */
    char* operator;          /* Operator (e.g., +, -, *, /) */
    int value;               /* Integer value */
    float float_value;       /* Float value */
    char* string;            /* Identifier name */

    /* Children nodes */
    ASTNode* left;           /* Left child */
    ASTNode* right;          /* Right child */
    ASTNode* next;           /* Sibling node (for lists) */

    /* For control structures */
    ASTNode* condition;     /* Condition expression */
    ASTNode* body;          /* Body of if/while */

    /* For functions */
    ASTNode* parameters;    /* Parameters list */
    ASTNode* arguments;     /* Arguments list */

    /* For arrays */
    int* array_sizes;       /* Sizes for each dimension */
    int dimensions;         /* Number of dimensions */

    /* Additional fields as needed */
};

/* Function Prototypes */

/* Create a new AST node */
ASTNode* create_ast_node(ASTNodeType type);

/* Create a new expression node */
ASTNode* create_expression_node(char* operator, ASTNode* left, ASTNode* right);

/* Add a child node */
void add_child(ASTNode* parent, ASTNode* child);

/* Add a sibling node */
void add_sibling(ASTNode* first, ASTNode* sibling);

/* Print the AST for debugging */
void print_ast(ASTNode* root, int level);

/* Free the AST */
void free_ast(ASTNode* root);

#endif /* AST_H */

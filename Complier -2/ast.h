/* ast.h */

#ifndef AST_H
#define AST_H

#include <stdbool.h>

// Maximum number of dimensions for arrays
#define MAX_DIMENSIONS 10

typedef struct ASTNode {
    char *node_type;       // Type of the node (e.g., "Program", "VarDecl", "Assignment", etc.)
    char *value;           // Value associated with the node (e.g., variable name, operator, literal)
    struct ASTNode *left;  // Left child
    struct ASTNode *right; // Right sibling
    int num_dimensions;    // Number of dimensions (for arrays)
    int sizes[MAX_DIMENSIONS]; // Sizes for each dimension
} ASTNode;

// Function declarations

// Helper function to create a new AST node
ASTNode* create_node(const char* type, const char* value);

// Create Program node
ASTNode *create_program_node(ASTNode *function_definitions, ASTNode *main_function);

// Add to Function Definitions List
ASTNode *add_to_function_list(ASTNode *list, ASTNode *func_def);

// Function Definition Nodes
ASTNode *create_function_def_node(const char *return_type, const char *func_name, ASTNode *body);

// Main Function Node
ASTNode *create_main_function_node(ASTNode *body);

// Function Body Nodes
ASTNode *create_function_body_node(ASTNode *declarations, ASTNode *statements, ASTNode *return_stmt);

// Main Body Nodes
ASTNode *create_main_body_node(ASTNode *declarations, ASTNode *statements, ASTNode *return_stmt);

// Return Statement Nodes
ASTNode *create_return_node(ASTNode *expr);

// Function Call Nodes
ASTNode *create_function_call_node(const char *func_name);

// Variable Declaration Nodes
ASTNode* create_var_decl_node(const char* type, const char* var_name, ASTNode* array_sizes, ASTNode* initializer);

// Array Declaration Nodes
ASTNode *create_array_decl_node(const char *type, const char *array_name, ASTNode *array_sizes, ASTNode *initializer);

// Assignment Nodes
ASTNode *create_assignment_node(const char *var_name, ASTNode *expr);
ASTNode *create_array_assignment_node(const char *array_name, ASTNode *index, ASTNode *expr);

// Write Statement Nodes
ASTNode *create_write_node(const char *var_name);
ASTNode *create_write_node_with_access(const char *array_name, ASTNode *index);

// Binary Operation Nodes
ASTNode *create_binary_op_node(const char *op, ASTNode *left, ASTNode *right);

// Number Nodes
ASTNode *create_number_node(int value);
ASTNode *create_float_node(float value);

// Identifier Node
ASTNode *create_id_node(const char *name);

// Array Access Node
ASTNode *create_array_access_node(const char *array_name, ASTNode *index);

// Array Initialization Nodes
ASTNode* create_initial_expression_node(ASTNode* expr);
ASTNode* add_to_initializer_list(ASTNode* list, ASTNode* expr);
ASTNode* create_array_init_node(const char* array_name, ASTNode* initializer_list);

// Add to Array Sizes List (for multi-dimensional arrays)
ASTNode *add_to_array_sizes_list(ASTNode *list, ASTNode *size);

// Add to Variable Declaration List
ASTNode *add_to_var_decl_list(ASTNode *list, ASTNode *decl);

// Add to Statement List
ASTNode *add_to_stmt_list(ASTNode *list, ASTNode *stmt);

// Traverse and Print the AST with Indentation
void traverse_ast(ASTNode *root, int depth);

// Free the AST
void free_ast(ASTNode *root);

#endif /* AST_H */

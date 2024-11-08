/* ast.c */

#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper function to create a new AST node
ASTNode* create_node(const char* type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for ASTNode.\n");
        exit(1);
    }
    node->node_type = strdup(type);
    node->value = value ? strdup(value) : NULL;
    node->left = NULL;
    node->right = NULL;
    node->num_dimensions = 0;
    memset(node->sizes, 0, sizeof(node->sizes));
    return node;
}

/* Program Node */
ASTNode *create_program_node(ASTNode *function_definitions, ASTNode *main_function)
{
    ASTNode *program = create_node("Program", NULL);
    program->left = function_definitions; // FunctionDefList
    program->right = main_function;       // MainFunction
    return program;
}

/* Add to Function Definitions List */
ASTNode *add_to_function_list(ASTNode *list, ASTNode *func_def)
{
    if (list == NULL)
    {
        return func_def;
    }
    ASTNode *current = list;
    while (current->right != NULL)
    {
        current = current->right;
    }
    current->right = func_def;
    return list;
}

/* Function Definition Nodes */
ASTNode *create_function_def_node(const char *return_type, const char *func_name, ASTNode *body)
{
    ASTNode *func_def = create_node("FunctionDef", func_name);

    // Return Type node
    ASTNode *return_type_node = create_node("ReturnType", return_type);
    func_def->left = return_type_node;

    // Function Body node
    func_def->right = body;

    return func_def;
}

ASTNode *create_main_function_node(ASTNode *body)
{
    ASTNode *main_func = create_node("MainFunction", "main");

    // Function Body node
    main_func->left = body;

    return main_func;
}

/* Function Body Nodes */
ASTNode *create_function_body_node(ASTNode *declarations, ASTNode *statements, ASTNode *return_stmt) {
    ASTNode *func_body = create_node("FunctionBody", NULL);
    func_body->left = declarations; // VarDeclList

    // Attach the return statement at the end of the statements
    if (statements == NULL) {
        statements = return_stmt;
    } else {
        ASTNode *current = statements;
        while (current->right != NULL) {
            current = current->right;
        }
        current->right = return_stmt;
    }

    func_body->right = statements; // Statements and Return
    return func_body;
}

ASTNode *create_main_body_node(ASTNode *declarations, ASTNode *statements, ASTNode *return_stmt)
{
    ASTNode *main_body = create_node("MainBody", NULL);
    main_body->left = declarations;
    main_body->right = statements;

    // Attach return statement as a sibling to statements
    ASTNode *current = statements;
    if (current != NULL)
    {
        while (current->right != NULL)
        {
            current = current->right;
        }
        current->right = return_stmt;
    }
    else
    {
        main_body->right = return_stmt;
    }

    return main_body;
}

/* Return Statement Nodes */
ASTNode *create_return_node(ASTNode *expr)
{
    ASTNode *return_node = create_node("Return", NULL);
    return_node->left = expr;
    return return_node;
}

/* Function Call Nodes */
ASTNode *create_function_call_node(const char *func_name)
{
    ASTNode *func_call = create_node("FunctionCall", func_name);
    return func_call;
}

/* Variable Declaration Nodes */
ASTNode* create_var_decl_node(const char* type, const char* var_name, ASTNode* array_sizes, ASTNode* initializer) {
    ASTNode* var_decl = create_node("VarDecl", var_name); // Ensure var_name is passed correctly
    var_decl->left = array_sizes; // For arrays
    var_decl->right = initializer; // For initialization
    return var_decl;
}

ASTNode *create_array_decl_node(const char *type, const char *array_name, ASTNode *array_sizes, ASTNode *initializer)
{
    // Create ArrayDecl node
    ASTNode *array_decl = create_node("ArrayDecl", array_name);

    // Type node
    ASTNode *type_node = create_node("Type", type);
    array_decl->left = type_node;

    // Array Sizes node (if any)
    if (array_sizes)
    {
        ASTNode *sizes_node = create_node("ArraySizes", NULL);
        sizes_node->left = array_sizes;
        array_decl->right = sizes_node;
    }

    // Initializer node (if any)
    if (initializer)
    {
        ASTNode *initializer_node = create_node("Initializer", NULL);
        if (array_sizes)
        {
            ASTNode *sizes_node = array_decl->right; // Navigate to ArraySizes node
            sizes_node->right = initializer_node;
        }
        else
        {
            array_decl->right = initializer_node;
        }
        initializer_node->left = initializer;
    }

    return array_decl;
}

/* Assignment Nodes */
ASTNode *create_assignment_node(const char *var_name, ASTNode *expr)
{
    ASTNode *assign = create_node("Assignment", var_name);
    assign->left = expr;
    return assign;
}

ASTNode *create_array_assignment_node(const char *array_name, ASTNode *index, ASTNode *expr)
{
    ASTNode *array_assign = create_node("ArrayAssignment", array_name);
    array_assign->left = index;
    array_assign->right = expr;
    return array_assign;
}

/* Write Statement Nodes */
ASTNode *create_write_node(const char *var_name)
{
    ASTNode *write = create_node("Write", var_name);
    return write;
}

ASTNode *create_write_node_with_access(const char *array_name, ASTNode *index)
{
    ASTNode *write = create_node("Write", array_name);
    ASTNode *access = create_array_access_node(array_name, index);
    write->left = access;
    return write;
}

/* Binary Operation Nodes */
ASTNode *create_binary_op_node(const char *op, ASTNode *left, ASTNode *right)
{
    ASTNode *bin_op = create_node("BinaryOp", op);
    bin_op->left = left;
    bin_op->right = right;
    return bin_op;
}

/* Number Nodes */
ASTNode *create_number_node(int value)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return create_node("Number", buffer);
}

ASTNode *create_float_node(float value)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%.2f", value);
    return create_node("Float", buffer);
}

/* Identifier Node */
ASTNode *create_id_node(const char *name)
{
    return create_node("ID", name);
}

/* Array Access Node */
ASTNode *create_array_access_node(const char *array_name, ASTNode *index)
{
    ASTNode *array_access = create_node("ArrayAccess", array_name);
    array_access->left = index;
    return array_access;
}

/* Array Initialization Nodes */

// Create Initializer node (for array initializations)
ASTNode* create_initial_expression_node(ASTNode* expr) {
    ASTNode* initializer = create_node("Initializer", NULL); // Initializer doesn't need a value
    initializer->left = expr;
    initializer->right = NULL;
    return initializer;
}

// Add to Initializer List (Corrected)
ASTNode* add_to_initializer_list(ASTNode* list, ASTNode* expr) {
    if (list == NULL) {
        // Start the initializer list with the first Initializer node
        return create_initial_expression_node(expr);
    }
    ASTNode* current = list;
    while (current->right != NULL) {
        current = current->right;
    }
    // Append the new Initializer node to the list
    current->right = create_initial_expression_node(expr);
    return list;
}

// Create Array Initialization node with array name
ASTNode* create_array_init_node(const char* array_name, ASTNode* initializer_list) {
    ASTNode* node = create_node("ArrayInit", array_name); // Store array name
    node->left = initializer_list;    // Initializer list
    node->right = NULL;
    node->num_dimensions = 0;
    memset(node->sizes, 0, sizeof(node->sizes));
    return node;
}

/* Add to Array Sizes List (for multi-dimensional arrays) */
ASTNode *add_to_array_sizes_list(ASTNode *list, ASTNode *size)
{
    if (list == NULL)
    {
        // Initialize a new list with the first size
        list = size;
        list->num_dimensions = 1;
        list->sizes[0] = atoi(size->value);
        return list;
    }
    // Traverse to the end of the list
    ASTNode *current = list;
    while (current->right != NULL)
    {
        current = current->right;
    }
    // Append the new size
    current->right = size;
    current->num_dimensions += 1;
    if (current->num_dimensions <= MAX_DIMENSIONS)
    {
        current->sizes[current->num_dimensions - 1] = atoi(size->value);
    }
    else
    {
        fprintf(stderr, "Error: Exceeded maximum array dimensions (%d).\n", MAX_DIMENSIONS);
        exit(1);
    }
    return list;
}

/* Add to Variable Declaration List */
ASTNode *add_to_var_decl_list(ASTNode *list, ASTNode *decl)
{
    if (decl == NULL)
    {
        // Do not add NULL declarations
        return list;
    }

    if (list == NULL)
    {
        // Initialize the list with the first VarDecl
        ASTNode *var_decl_list = create_node("VarDeclList", NULL);
        var_decl_list->left = decl;
        var_decl_list->right = NULL;
        return var_decl_list;
    }

    // Traverse to the end of the VarDeclList
    ASTNode *current = list;
    while (current->right != NULL)
    {
        current = current->right;
    }

    // Create a new VarDeclList node and append it
    ASTNode *new_var_decl_list = create_node("VarDeclList", NULL);
    new_var_decl_list->left = decl;
    new_var_decl_list->right = NULL;
    current->right = new_var_decl_list;

    return list;
}

/* Add to Statement List */
ASTNode *add_to_stmt_list(ASTNode *list, ASTNode *stmt)
{
    if (list == NULL)
    {
        return stmt;
    }
    ASTNode *current = list;
    while (current->right != NULL)
    {
        current = current->right;
    }
    current->right = stmt;
    return list;
}

/* Traverse and Print the AST with Indentation */
void traverse_ast(ASTNode *root, int depth)
{
    if (root == NULL)
        return;

    // Print indentation
    for (int i = 0; i < depth; i++)
        printf("  ");

    // Print node type and value
    if (root->value)
        printf("%s: %s\n", root->node_type, root->value);
    else
        printf("%s\n", root->node_type);

    // Traverse left child (first child)
    traverse_ast(root->left, depth + 1);

    // Traverse right sibling (next sibling)
    traverse_ast(root->right, depth);
}

/* Free the AST */
void free_ast(ASTNode *root)
{
    if (root == NULL)
        return;
    free_ast(root->left);
    free_ast(root->right);
    free(root->node_type);
    if (root->value)
        free(root->value);
    free(root);
}

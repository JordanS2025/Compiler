/* semantic.c */

#include "semantic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Function to perform semantic analysis by traversing the AST */
bool perform_semantic_analysis(ASTNode *root, SymbolTable *sym_table) {
    if (root == NULL)
        return true; // Changed to return true for consistency

    // Perform analysis based on node type
    if (strcmp(root->node_type, "Program") == 0) {
        perform_semantic_analysis(root->left, sym_table);  // Function Definitions
        perform_semantic_analysis(root->right, sym_table); // Main Function
    }
    else if (strcmp(root->node_type, "FunctionDef") == 0) {
        // Traverse function body
        perform_semantic_analysis(root->right, sym_table);
    }
    else if (strcmp(root->node_type, "MainFunction") == 0) {
        // Traverse main function body
        perform_semantic_analysis(root->left, sym_table);
    }
    else if (strcmp(root->node_type, "FunctionBody") == 0 ||
             strcmp(root->node_type, "MainBody") == 0) {
        perform_semantic_analysis(root->left, sym_table);  // Declarations
        perform_semantic_analysis(root->right, sym_table); // Statements and Return
    }
    else if (strcmp(root->node_type, "VarDecl") == 0) {
        // Retrieve the variable's symbol from the symbol table
        Symbol *sym = get_symbol(sym_table, root->value);
        if (sym == NULL) {
            fprintf(stderr, "Semantic Error: Undeclared variable '%s'.\n", root->value);
            exit(1);
        }
        const char *var_type = sym->type;

        // Check if it's an array declaration by inspecting the left child for "ArraySizes"
        if (root->left && strcmp(root->left->node_type, "ArraySizes") == 0) {
            // Array declaration with initializer
            ASTNode *array_sizes_node = root->left; // ArraySizes node
            ASTNode *initializer_node = root->right;  // Initializer node
            if (initializer_node && strcmp(initializer_node->node_type, "Initializer") == 0) {
                ASTNode *array_init = initializer_node->left; // Array initialization list
                check_array_type_consistency(array_init, var_type, sym_table);
            }
            else {
                // Array declared without initializer
                fprintf(stderr, "Semantic Error: Array '%s' must be initialized at declaration.\n", root->value);
                exit(1);
            }
        }
        else {
            // Scalar variable declaration with initializer
            ASTNode *initializer = root->right; // Initializer node
            if (initializer && strcmp(initializer->node_type, "Initializer") == 0) {
                ASTNode *expr = initializer->left; // Expression node
                const char *expr_type = get_expression_type(expr, sym_table);

                // Check type consistency
                if (strcmp(var_type, "int") == 0 && strcmp(expr_type, "int") != 0) {
                    fprintf(stderr, "Semantic Error: Cannot assign '%s' expression to int variable '%s'\n",
                            expr_type, root->value);
                    exit(1);
                }
                else if (strcmp(var_type, "char") == 0 && strcmp(expr_type, "char") != 0) {
                    fprintf(stderr, "Semantic Error: Cannot assign '%s' expression to char variable '%s'\n",
                            expr_type, root->value);
                    exit(1);
                }
                // For float variables, mixed types are allowed (int can be promoted to float)
                // No action needed as per the requirement
            }
            else {
                // Variable declared without initializer
                // Depending on language rules, handle accordingly
                // For this scenario, variables must be initialized at declaration
                fprintf(stderr, "Semantic Error: Variable '%s' must be initialized at declaration.\n", root->value);
                exit(1);
            }
        }
    }
    else if (strcmp(root->node_type, "Assignment") == 0 ||
             strcmp(root->node_type, "ArrayAssignment") == 0) {
        // For assignments, ensure the expression type matches the variable's type
        const char *var_name;
        const char *var_type;
        if (strcmp(root->node_type, "Assignment") == 0) {
            var_name = root->value;
            Symbol *sym = get_symbol(sym_table, var_name);
            var_type = sym->type;
        }
        else { // ArrayAssignment
            var_name = root->value;
            Symbol *sym = get_symbol(sym_table, var_name);
            var_type = sym->type;
        }

        // Determine the type of the expression
        const char *expr_type = get_expression_type(root->left, sym_table);

        // Check type consistency
        if (strcmp(var_type, "int") == 0 && strcmp(expr_type, "int") != 0) {
            fprintf(stderr, "Semantic Error: Cannot assign '%s' expression to int variable '%s'\n",
                    expr_type, var_name);
            exit(1);
        }
        else if (strcmp(var_type, "char") == 0 && strcmp(expr_type, "char") != 0) {
            fprintf(stderr, "Semantic Error: Cannot assign '%s' expression to char variable '%s'\n",
                    expr_type, var_name);
            exit(1);
        }
        // For float variables, mixed types are allowed (int can be promoted to float)
        // No action needed as per the requirement
    }
    else if (strcmp(root->node_type, "BinaryOp") == 0) {
        // Ensure operation type consistency
        const char *left_type = get_expression_type(root->left, sym_table);
        const char *right_type = get_expression_type(root->right, sym_table);

        // If either operand is float, the result is float
        // If both are int, the result is int
        // No specific semantic rules needed here as type determination is handled in get_expression_type
    }
    else if (strcmp(root->node_type, "Write") == 0) {
        // Ensure the variable being written has a consistent type
        // No specific semantic rules provided for write operations
    }

    // Recursively analyze child nodes
    perform_semantic_analysis(root->left, sym_table);
    perform_semantic_analysis(root->right, sym_table);
}

/* Function to determine the type of an expression */
const char* get_expression_type(ASTNode *node, SymbolTable *sym_table) {
    if (node == NULL) {
        fprintf(stderr, "Semantic Error: Null expression node.\n");
        exit(1);
    }

    if (strcmp(node->node_type, "Number") == 0) {
        return "int";
    }
    else if (strcmp(node->node_type, "Float") == 0) {
        return "float";
    }
    else if (strcmp(node->node_type, "ID") == 0) {
        Symbol *sym = get_symbol(sym_table, node->value);
        if (!sym) {
            fprintf(stderr, "Semantic Error: Undeclared identifier '%s'.\n", node->value);
            exit(1);
        }
        return sym->type;
    }
    else if (strcmp(node->node_type, "ArrayAccess") == 0) {
        Symbol *sym = get_symbol(sym_table, node->value);
        if (!sym) {
            fprintf(stderr, "Semantic Error: Undeclared array '%s'.\n", node->value);
            exit(1);
        }
        return sym->type;
    }
    else if (strcmp(node->node_type, "BinaryOp") == 0) {
        const char *left_type = get_expression_type(node->left, sym_table);
        const char *right_type = get_expression_type(node->right, sym_table);

        if (strcmp(left_type, "float") == 0 || strcmp(right_type, "float") == 0) {
            return "float";
        }
        else {
            return "int";
        }
    }
    else if (strcmp(node->node_type, "FunctionCall") == 0) {
        Symbol *sym = get_symbol(sym_table, node->value);
        if (!sym) {
            fprintf(stderr, "Semantic Error: Undeclared function '%s'.\n", node->value);
            exit(1);
        }
        return sym->type;
    }
    else if (strcmp(node->node_type, "ArrayInit") == 0) {
        // Determine the type based on the first element
        if (node->left == NULL) {
            fprintf(stderr, "Semantic Error: Empty array initialization.\n");
            exit(1);
        }
        return get_expression_type(node->left, sym_table);
    }
    else if (strcmp(node->node_type, "Initializer") == 0) {
        // Delegate to the child expression
        return get_expression_type(node->left, sym_table);
    }
    else {
        fprintf(stderr, "Semantic Error: Unknown expression node type '%s'.\n", node->node_type);
        exit(1);
    }
}


/* Function to check array type consistency in array initialization */
void check_array_type_consistency(ASTNode *array_init, const char *expected_type, SymbolTable *sym_table) {
    if (array_init == NULL) {
        fprintf(stderr, "Semantic Error: Null array initialization.\n");
        exit(1);
    }

    // 'array_init' is the ArrayInit node
    // 'array_init->left' is the first expression
    ASTNode *first_expr = array_init->left;
    if (first_expr == NULL) {
        fprintf(stderr, "Semantic Error: Empty array initialization.\n");
        exit(1);
    }

    const char *first_elem_type = get_expression_type(first_expr, sym_table);

    if (strcmp(first_elem_type, expected_type) != 0) {
        fprintf(stderr, "Semantic Error: Type mismatch in array initialization. Expected '%s' but found '%s'.\n",
                expected_type, first_elem_type);
        exit(1);
    }

    // Traverse the initializer list via 'right' pointers starting from first_expr->right
    ASTNode *current = first_expr->right;
    while (current != NULL) {
        const char *elem_type = get_expression_type(current, sym_table);
        if (strcmp(elem_type, expected_type) != 0) {
            fprintf(stderr, "Semantic Error: Mixed types in array initialization. Expected '%s' but found '%s'.\n",
                    expected_type, elem_type);
            exit(1);
        }
        current = current->right;
    }
}

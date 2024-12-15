/* ast.c */

#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Create a new AST node with the specified type */
ASTNode* create_ast_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Failed to allocate memory for AST node.\n");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->name = NULL;
    node->data_type = DT_VOID;
    node->category = SYMBOL_VARIABLE;
    node->operator = NULL;
    node->value = 0;
    node->float_value = 0.0f;
    node->string = NULL;

    node->left = NULL;
    node->right = NULL;
    node->next = NULL;

    node->condition = NULL;
    node->body = NULL;

    node->parameters = NULL;
    node->arguments = NULL;

    node->array_sizes = NULL;
    node->dimensions = 0;
    return node;
}

/* Create a new expression node */
ASTNode* create_expression_node(char* operator, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_ast_node(AST_EXPRESSION);
    node->operator = strdup(operator);
    node->left = left;
    node->right = right;
    return node;
}

/* Add a child node to a parent */
void add_child(ASTNode* parent, ASTNode* child) {
    if (!parent->left) {
        parent->left = child;
    } else {
        ASTNode* temp = parent->left;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = child;
    }
}

/* Add a sibling node */
void add_sibling(ASTNode* first, ASTNode* sibling) {
    ASTNode* temp = first;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = sibling;
}

/* Helper function to print indentation */
void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

/* Recursive function to print the AST */
void print_ast(ASTNode* root, int level) {
    if (!root) return;

    /* Indentation */
    print_indent(level);

    /* Print node information based on type */
    switch(root->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_FUNCTION_DEFINITION:
            printf("Function Definition: %s\n", root->name);
            break;
        case AST_MAIN_FUNCTION:
            printf("Function Definition: main\n");
            break;
        case AST_DECLARATION:
            if (root->category == SYMBOL_ARRAY) {
                printf("Declaration: %s (int[%d])\n", root->name, root->array_sizes[0]);
            } else {
                printf("Declaration: %s (%s)\n", root->name, 
                       root->data_type == DT_INT ? "int" :
                       root->data_type == DT_FLOAT ? "float" :
                       root->data_type == DT_CHAR ? "char" : "unknown");
            }
            break;
        case AST_ASSIGNMENT:
            printf("Assignment Statement: %s\n", root->name);
            break;
        case AST_WRITE:
            printf("Write Statement: ");
            if (root->left && root->left->type == AST_ARRAY_ACCESS) {
                /* Handle writing to array elements */
                printf("%s[%d]\n", root->left->string, root->left->left->value);
            } else if (root->name) {
                printf("ID(%s)\n", root->name);
            } else {
                printf("Unknown\n");
            }
            break;
        case AST_IF:
            printf("If Statement\n");
            break;
        case AST_WHILE:
            printf("While Statement\n");
            break;
        case AST_RETURN:
            printf("Return Statement\n");
            break;
        case AST_EXPRESSION:
            if (root->operator) {
                if (strcmp(root->operator, "ID") == 0) {
                    printf("Expression: ID(%s)\n", root->string);
                } else if (strcmp(root->operator, "NUMBER") == 0) {
                    printf("Expression: NUMBER(%d)\n", root->value);
                } else if (strcmp(root->operator, "FLOAT_NUMBER") == 0) {
                    printf("Expression: FLOAT_NUMBER(%.2f)\n", root->float_value);
                } else {
                    printf("Expression: %s\n", root->operator);
                }
            } else if (root->string) {
                printf("Expression: ID(%s)\n", root->string);
            } else {
                printf("Expression: Unknown\n");
            }
            break;
        case AST_BLOCK:
            printf("Block\n");
            break;
        case AST_ARRAY_INIT:
            printf("Array Initialization\n");
            break;
        case AST_FUNCTION_CALL:
            printf("Function Call: %s\n", root->string);
            break;
        case AST_ARRAY_ACCESS:
            /* This case is handled within AST_WRITE */
            break;
        /* Add more cases as needed */
        default:
            printf("Unknown AST Node\n");
    }

    /* Special handling for certain node types */
    switch(root->type) {
        case AST_FUNCTION_DEFINITION:
        case AST_MAIN_FUNCTION:
        case AST_BLOCK:
            if (root->left) {
                print_ast(root->left, level + 1);
            }
            break;
        case AST_DECLARATION:
            if (root->category == SYMBOL_ARRAY && root->left) {
                /* For array initialization */
                print_indent(level + 1);
                printf("Array Initialization\n");
                print_ast(root->left, level + 2);
            } else if (root->left) {
                /* For initialized variables */
                print_indent(level + 1);
                printf("Initialization:\n");
                print_ast(root->left, level + 2);
            }
            break;
        case AST_ASSIGNMENT:
            if (root->left) {
                print_indent(level + 1);
                printf("Expression:\n");
                print_ast(root->left, level + 2);
            }
            break;
        case AST_IF:
            if (root->condition) {
                print_indent(level + 1);
                printf("Condition:\n");
                print_ast(root->condition, level + 2);
            }
            if (root->body) {
                print_indent(level + 1);
                printf("Block\n");
                print_ast(root->body, level + 2);
            }
            break;
        case AST_WHILE:
            if (root->condition) {
                print_indent(level + 1);
                printf("Condition:\n");
                print_ast(root->condition, level + 2);
            }
            if (root->body) {
                print_indent(level + 1);
                printf("Block\n");
                print_ast(root->body, level + 2);
            }
            break;
        case AST_RETURN:
            if (root->left) {
                print_indent(level + 1);
                printf("Expression:\n");
                print_ast(root->left, level + 2);
            }
            break;
        case AST_FUNCTION_CALL:
            if (root->arguments) {
                print_indent(level + 1);
                printf("Arguments:\n");
                print_ast(root->arguments, level + 2);
            }
            break;
        case AST_ARRAY_ACCESS:
            /* Not directly printed; handled within AST_WRITE */
            break;
        default:
            /* For other node types, process children normally */
            if (root->left) {
                print_ast(root->left, level + 1);
            }
            if (root->right) {
                print_ast(root->right, level + 1);
            }
            break;
    }

    /* Print siblings */
    if (root->next) {
        print_ast(root->next, level);
    }
}

/* Recursive function to free the AST */
void free_ast(ASTNode* root) {
    if (!root) return;

    /* Free current node's data */
    if (root->name) free(root->name);
    if (root->operator) free(root->operator);
    if (root->string) free(root->string);
    if (root->array_sizes) free(root->array_sizes);

    /* Free children */
    if (root->left) free_ast(root->left);
    if (root->right) free_ast(root->right);
    if (root->condition) free_ast(root->condition);
    if (root->body) free_ast(root->body);
    if (root->parameters) free_ast(root->parameters);
    if (root->arguments) free_ast(root->arguments);

    /* Free siblings */
    if (root->next) free_ast(root->next);

    /* Free the node itself */
    free(root);
}

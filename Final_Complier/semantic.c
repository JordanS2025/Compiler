#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "ast.h"
#include "symbol_table.h"

/* Global counter for semantic errors */
static int semantic_error_count = 0;

/* Forward declarations of helper functions */
static void traverse_node(ASTNode* node);
static DataType deduce_type_from_operator(const char* op, DataType left_type, DataType right_type);
static void check_condition(ASTNode* cond_node);
static int count_initializers(ASTNode* init_node);

/* Report semantic errors with line number information if available (assuming a global line_num) */
extern int line_num;

void report_semantic_error(const char* format, ...) {
    va_list args;
    fprintf(stderr, "Semantic error at line %d: ", line_num);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    semantic_error_count++;
}

void traverse_ast(ASTNode* root) {
    if (!root) return;
    traverse_node(root);

    /* After traversal, if semantic_error_count > 0, we may want to stop code generation */
    if (semantic_error_count > 0) {
        fprintf(stderr, "%d semantic error(s) found. Compilation halted.\n", semantic_error_count);
        exit(EXIT_FAILURE);
    }
}

static void traverse_node(ASTNode* node) {
    if (!node) return;

    switch(node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            /* Traverse children */
            for (ASTNode* child = node->left; child; child = child->next) {
                traverse_node(child);
            }
            break;
        
        case AST_FUNCTION_DEFINITION:
        case AST_MAIN_FUNCTION:
            /* Traverse children (parameters, body) */
            for (ASTNode* child = node->left; child; child = child->next) {
                traverse_node(child);
            }
            break;

        case AST_DECLARATION:
            /* If it's an array declaration with initialization, check that */
            if (node->category == SYMBOL_ARRAY && node->left && node->type == AST_DECLARATION) {
                check_array_initialization(node);
            }

            /* If it has an initialization expression, check it */
            if (node->left && node->category == SYMBOL_VARIABLE) {
                check_expression(node->left);
            }

            /* Traverse any children */
            for (ASTNode* child = node->left; child; child = child->next) {
                traverse_node(child);
            }
            break;

        case AST_ASSIGNMENT:
            /* Check the expression on the right-hand side */
            if (node->left) {
                DataType rhs_type = check_expression(node->left);

                /* Check that the variable being assigned exists and types match */
                Symbol* sym = lookup_symbol(node->name);
                if (!sym) {
                    report_semantic_error("Assignment to undeclared variable '%s'", node->name);
                } else {
                    /* If symbol found, check type compatibility */
                    if ((sym->type == DT_INT || sym->type == DT_FLOAT || sym->type == DT_CHAR) && rhs_type != sym->type) {
                        report_semantic_error("Type mismatch in assignment to '%s'. Expected '%s', got '%s'",
                            node->name,
                            (sym->type == DT_INT ? "int" : (sym->type == DT_FLOAT ? "float" : "char")),
                            (rhs_type == DT_INT ? "int" : (rhs_type == DT_FLOAT ? "float" : "char")));
                    }
                }
            }
            break;

        case AST_WRITE:
            /* Just ensure the symbol exists. Type checks are simple here. */
            if (node->name) {
                Symbol* sym = lookup_symbol(node->name);
                if (!sym) {
                    report_semantic_error("Write statement references undeclared variable '%s'", node->name);
                }
            }
            /* If it's writing array element */
            if (node->left && node->left->type == AST_ARRAY_ACCESS) {
                /* Check index type is int */
                if (node->left->left) {
                    DataType idx_type = check_expression(node->left->left);
                    if (idx_type != DT_INT) {
                        report_semantic_error("Array index must be int type.");
                    }
                }
            }
            /* No need to further check */
            break;

        case AST_IF:
        case AST_WHILE:
            /* Check condition */
            if (node->condition) {
                check_condition(node->condition);
            }
            /* Traverse body */
            if (node->body) {
                traverse_node(node->body);
            }
            /* If there is an else or additional children, traverse them */
            for (ASTNode* c = node->left; c; c = c->next) {
                traverse_node(c);
            }
            break;

        case AST_RETURN:
            /* Check return expression type */
            if (node->left) {
                check_expression(node->left);
            }
            break;

        case AST_EXPRESSION:
        case AST_FUNCTION_CALL:
        case AST_ARRAY_INIT:
        case AST_ARRAY_ACCESS:
            /* Expression-related nodes are checked by check_expression when needed */
            /* But we can do a general traversal */
            if (node->left) traverse_node(node->left);
            if (node->right) traverse_node(node->right);
            for (ASTNode* c = node->left; c; c = c->next) {
                traverse_node(c);
            }
            break;

        default:
            /* For unknown or not handled explicitly, just traverse children */
            if (node->left) traverse_node(node->left);
            if (node->right) traverse_node(node->right);
            for (ASTNode* c = node->left; c; c = c->next) {
                traverse_node(c);
            }
            break;
    }
}

/* Check semantic correctness of conditions in if/while.
   Conditions usually are comparisons like ==, !=, <, >, etc. */
static void check_condition(ASTNode* cond_node) {
    DataType cond_type = check_expression(cond_node);
    /* If condition_type is from a comparison operator, it should end up boolean-like,
       but we have no explicit boolean type. The user asked for same-type operands in comparison.
       Already handled in check_expression. Just ensure we got a known type. */
    if (cond_type == DT_VOID) {
        report_semantic_error("Invalid condition type in if/while statement.");
    }
}

/* Check array initialization matches the declared size */
void check_array_initialization(ASTNode* declaration_node) {
    if (!declaration_node || declaration_node->category != SYMBOL_ARRAY) return;

    Symbol* sym = lookup_symbol(declaration_node->name);
    if (!sym || sym->category != SYMBOL_ARRAY) return;

    int declared_size = 1;
    for (int i = 0; i < sym->dimensions; i++) {
        declared_size *= sym->array_sizes[i];
    }

    if (declaration_node->left && declaration_node->left->type == AST_ARRAY_INIT) {
        int init_count = count_initializers(declaration_node->left->left);
        if (init_count > declared_size) {
            report_semantic_error("Array '%s' initialized with too many elements. Declared size: %d, Provided: %d",
                                  declaration_node->name, declared_size, init_count);
        }
    }
}

/* Count the number of elements in an initializer list */
static int count_initializers(ASTNode* init_node) {
    int count = 0;
    for (ASTNode* c = init_node; c; c = c->next) {
        count++;
    }
    return count;
}

/* Check expressions recursively and return their resulting type */
DataType check_expression(ASTNode* expr) {
    if (!expr) return DT_VOID;

    switch (expr->type) {
        case AST_EXPRESSION:
            /* If operator is ID, NUMBER, FLOAT_NUMBER, or a unary operator */
            if (expr->operator) {
                if (strcmp(expr->operator, "ID") == 0) {
                    /* Lookup symbol type */
                    Symbol* sym = lookup_symbol(expr->string);
                    if (!sym) {
                        report_semantic_error("Undeclared variable '%s' in expression.", expr->string);
                        return DT_VOID;
                    }
                    return sym->type;
                } else if (strcmp(expr->operator, "NUMBER") == 0) {
                    return DT_INT;
                } else if (strcmp(expr->operator, "FLOAT_NUMBER") == 0) {
                    return DT_FLOAT;
                } else if (strcmp(expr->operator, "!") == 0) {
                    /* NOT operator - typically boolean context, 
                       but we only have int/float/char. Let's assume it's int (0 or 1).
                       Check expression operand */
                    DataType t = check_expression(expr->left);
                    return t == DT_INT || t == DT_FLOAT || t == DT_CHAR ? DT_INT : DT_VOID;
                } else {
                    /* It's likely a binary operator like +, -, *, /, ||, &&, ==, !=, >, <, etc. */
                    DataType left_type = check_expression(expr->left);
                    DataType right_type = check_expression(expr->right);
                    return deduce_type_from_operator(expr->operator, left_type, right_type);
                }
            } else {
                /* No operator means it could be a simple variable ref? Already handled above */
                return DT_VOID;
            }
            break;

        case AST_FUNCTION_CALL: {
            /* Check function call return type */
            Symbol* sym = lookup_symbol(expr->string);
            if (!sym || sym->category != SYMBOL_FUNCTION) {
                report_semantic_error("Call to undeclared function '%s'.", expr->string);
                return DT_VOID;
            }
            /* For now, assume arguments are correct. Could add argument checks. */
            /* Check arguments */
            ASTNode* arg = expr->arguments;
            while (arg) {
                check_expression(arg);
                arg = arg->next;
            }
            return sym->return_type;  
        }

        case AST_ARRAY_ACCESS: {
            /* Check array symbol and index type */
            Symbol* sym = lookup_symbol(expr->string);
            if (!sym || sym->category != SYMBOL_ARRAY) {
                report_semantic_error("Invalid array access on '%s'. Not an array.", expr->string);
                return DT_VOID;
            }
            /* Check index is int */
            if (expr->left) {
                DataType idx_type = check_expression(expr->left);
                if (idx_type != DT_INT) {
                    report_semantic_error("Array index must be int type for '%s'.", expr->string);
                }
            }
            /* Array access results in the array's base type */
            /* The symbol might have type DT_ARRAY, but base type is stored in 'type' */
            /* In the provided code, arrays are stored as DT_ARRAY. Let's assume base type int for simplicity 
               or ideally we would store the element type separately. For now, we rely on the original type. */
            return sym->type == DT_ARRAY ? DT_INT : sym->type;
        }

        default:
            /* Other nodes not directly expressions */
            return DT_VOID;
    }
}

/* Deduce the resulting type from a binary operator and its operand types.
   Also checks for semantic errors when mixing int and float or comparing different types. */
static DataType deduce_type_from_operator(const char* op, DataType left_type, DataType right_type) {
    /* If either side is void, propagate void to avoid cascading errors */
    if (left_type == DT_VOID || right_type == DT_VOID) {
        return DT_VOID;
    }

    /* For arithmetic operators (+, -, *, /): 
       - Both must be int or both must be float, or both must be char?
       - The user wants an error if int is added to float (mixing types not allowed)
    */
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
        strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
        
        if (left_type != right_type) {
            report_semantic_error("Type mismatch in arithmetic operation '%s'. Left: %s, Right: %s", op,
                (left_type == DT_INT ? "int" : left_type == DT_FLOAT ? "float" : "char"),
                (right_type == DT_INT ? "int" : right_type == DT_FLOAT ? "float" : "char"));
            return DT_VOID;
        }
        /* If same type, return that type */
        return left_type;
    }

    /* Logical or comparison operators (==, !=, <, >, <=, >=):
       - Both operands must be of the same type.
    */
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
        
        if (left_type != right_type) {
            report_semantic_error("Type mismatch in comparison '%s'. Left: %s, Right: %s", op,
                (left_type == DT_INT ? "int" : left_type == DT_FLOAT ? "float" : "char"),
                (right_type == DT_INT ? "int" : right_type == DT_FLOAT ? "float" : "char"));
            return DT_VOID;
        }
        /* Comparison results in int (0 or 1 for boolean context) */
        return DT_INT;
    }

    /* Logical operators (&&, ||):
       - Typically these expect boolean (int) types, but since we don't have a boolean,
         assume int or treat float/char as error. For simplicity, let's allow int only.
    */
    if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
        if (left_type != DT_INT || right_type != DT_INT) {
            report_semantic_error("Logical operator '%s' requires integer operands.", op);
            return DT_VOID;
        }
        return DT_INT;
    }

    /* If we get here, operator is unknown */
    return DT_VOID;
}


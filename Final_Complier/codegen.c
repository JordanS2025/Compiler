#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"

/* File pointer for TAC output */
static FILE* tac_out = NULL;

/* Temporary and label counters */
static int temp_count = 0;
static int label_count = 0;

/* Forward declarations */
static void gen_node(ASTNode* node);
static char* gen_expression(ASTNode* expr);
static void gen_if(ASTNode* node);
static void gen_while(ASTNode* node);
static void gen_write(ASTNode* node);
static void gen_return(ASTNode* node);
static void gen_function_def(ASTNode* node);
static void gen_main_function(ASTNode* node);
static void gen_block(ASTNode* node);
static void gen_assignment(ASTNode* node);
static void gen_declaration(ASTNode* node);
static char* gen_function_call_expr(ASTNode* node);
static void gen_char_assignment(const char* var, char ch);
static char* gen_increment_expr(const char* var, int amount);

char* new_temp() {
    char* buf = (char*)malloc(16);
    sprintf(buf, "t%d", temp_count++);
    return buf;
}

char* new_label() {
    char* buf = (char*)malloc(16);
    sprintf(buf, "L%d", label_count++);
    return buf;
}

static char* gen_increment_expr(const char* var, int amount) {
    char* t1 = new_temp();
    char* t2 = new_temp();
    fprintf(tac_out, "%s = %s\n", t1, var);
    fprintf(tac_out, "%s = %s + %d\n", t2, t1, amount);
    fprintf(tac_out, "%s = %s\n", var, t2);
    free(t1);
    return t2;
}

void generate_tac(ASTNode* root) {
    /* Open the TAC file for writing */
    tac_out = fopen("tac_output.txt", "w");
    if (!tac_out) {
        fprintf(stderr, "Failed to open tac_output.txt for writing.\n");
        exit(1);
    }

    if (!root) {
        fclose(tac_out);
        return;
    }

    gen_node(root);

    fclose(tac_out);
}

static void gen_node(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (ASTNode* c = node->left; c; c = c->next) {
                gen_node(c);
            }
            break;
        case AST_FUNCTION_DEFINITION:
            gen_function_def(node);
            break;
        case AST_MAIN_FUNCTION:
            gen_main_function(node);
            break;
        case AST_IF:
            gen_if(node);
            break;
        case AST_WHILE:
            gen_while(node);
            break;
        case AST_WRITE:
            gen_write(node);
            break;
        case AST_RETURN:
            gen_return(node);
            break;
        case AST_BLOCK:
            gen_block(node);
            break;
        case AST_ASSIGNMENT:
            gen_assignment(node);
            break;
        case AST_DECLARATION:
            gen_declaration(node);
            break;
        case AST_FUNCTION_CALL:
            (void)gen_expression(node);
            break;
        default:
            if (node->left) gen_node(node->left);
            if (node->right) gen_node(node->right);
            for (ASTNode* c = node->left; c; c = c->next) {
                gen_node(c);
            }
            break;
    }
}

static void gen_block(ASTNode* node) {
    for (ASTNode* c = node->left; c; c = c->next) {
        gen_node(c);
    }
}

static void gen_if(ASTNode* node) {
    char* cond_reg = gen_expression(node->condition);
    char* else_label = new_label();
    fprintf(tac_out, "IFZ %s GOTO %s\n", cond_reg, else_label);
    free(cond_reg);

    if (node->body) {
        gen_node(node->body);
        if (node->body->next && node->body->next->type == AST_ASSIGNMENT) {
            gen_node(node->body->next);
        }
    }

    fprintf(tac_out, "%s:\n", else_label);
    free(else_label);
}

static void gen_while(ASTNode* node) {
    char* start_label = new_label();
    char* end_label = new_label();
    
    fprintf(tac_out, "%s:\n", start_label);
    char* cond_reg = gen_expression(node->condition);
    fprintf(tac_out, "IFZ %s GOTO %s\n", cond_reg, end_label);
    free(cond_reg);

    if (node->body) {
        gen_node(node->body);
        if (node->body->next && node->body->next->type == AST_ASSIGNMENT) {
            gen_node(node->body->next);
        }
    }
    
    fprintf(tac_out, "GOTO %s\n", start_label);
    fprintf(tac_out, "%s:\n", end_label);
    free(start_label);
    free(end_label);
}

static void gen_write(ASTNode* node) {
    char* val_reg = NULL;
    if (node->name && !node->left) {
        val_reg = new_temp();
        fprintf(tac_out, "%s = %s\n", val_reg, node->name);
    } else if (node->left && node->left->type == AST_ARRAY_ACCESS) {
        ASTNode* arrNode = node->left;
        char* idx = gen_expression(arrNode->left);
        val_reg = new_temp();
        fprintf(tac_out, "%s = %s[%s]\n", val_reg, arrNode->string, idx);
        free(idx);
    } else if (node->left) {
        val_reg = gen_expression(node->left);
    } else {
        val_reg = new_temp();
        fprintf(tac_out, "%s = 0\n", val_reg);
    }
    fprintf(tac_out, "WRITE %s\n", val_reg);
    free(val_reg);
}

static void gen_return(ASTNode* node) {
    if (node->left) {
        char* ret_reg = gen_expression(node->left);
        fprintf(tac_out, "RETURN %s\n", ret_reg);
        free(ret_reg);
    } else {
        fprintf(tac_out, "RETURN\n");
    }
}

static void gen_function_def(ASTNode* node) {
    fprintf(tac_out, "FUNC_BEGIN %s\n", node->name);

    // If function definition has parameters, assign them from param0, param1, etc.
    // Assuming ASTNode* node->params holds a linked list of parameter nodes, each with a name.
    // If your AST differs, adjust accordingly.
    ASTNode* p = node->parameters; 
    int paramIndex = 0;
    while (p) {
        // Assign parameter variable from paramN
        fprintf(tac_out, "%s = param%d\n", p->name, paramIndex);
        paramIndex++;
        p = p->next;
    }

    for (ASTNode* c = node->left; c; c = c->next) {
        gen_node(c);
    }
    fprintf(tac_out, "FUNC_END %s\n", node->name);
}

static void gen_main_function(ASTNode* node) {
    fprintf(tac_out, "FUNC_BEGIN main\n");
    if (node->left) gen_node(node->left);
    fprintf(tac_out, "FUNC_END main\n");
}

static void gen_assignment(ASTNode* node) {
    ASTNode* valNode = node->left;
    ASTNode* arrayNode = valNode ? valNode->next : NULL;

    if (arrayNode && arrayNode->type == AST_ARRAY_ACCESS) {
        char* val_reg = gen_expression(valNode);
        char* idx_reg = gen_expression(arrayNode->left);
        fprintf(tac_out, "%s[%s] = %s\n", node->name, idx_reg, val_reg);
        free(val_reg);
        free(idx_reg);
    } else if (valNode && valNode->type == AST_EXPRESSION && 
               valNode->operator && strcmp(valNode->operator, "+") == 0 &&
               valNode->left->type == AST_EXPRESSION && 
               valNode->left->operator && strcmp(valNode->left->operator, "ID") == 0 &&
               strcmp(valNode->left->string, node->name) == 0 &&
               valNode->right->type == AST_EXPRESSION && 
               valNode->right->operator && strcmp(valNode->right->operator, "NUMBER") == 0) {
        char* result = gen_increment_expr(node->name, valNode->right->value);
        free(result);
    } else {
        char* val_reg = gen_expression(valNode);
        fprintf(tac_out, "%s = %s\n", node->name, val_reg);
        free(val_reg);
    }
}

static void gen_declaration(ASTNode* node) {
    if (node->category == SYMBOL_VARIABLE && node->left) {
        if (node->data_type == DT_CHAR && node->left->operator && 
            strlen(node->left->operator) == 1 && node->left->operator[0] != 'N') {
            char ch = node->left->operator[0];
            gen_char_assignment(node->name, ch);
        } else {
            char* val_reg = gen_expression(node->left);
            fprintf(tac_out, "%s = %s\n", node->name, val_reg);
            free(val_reg);
        }
    } else if (node->category == SYMBOL_ARRAY && node->left && node->left->type == AST_ARRAY_INIT) {
        int idx = 0;
        for (ASTNode* init_expr = node->left->left; init_expr; init_expr = init_expr->next, idx++) {
            char* val_reg = gen_expression(init_expr);
            fprintf(tac_out, "%s[%d] = %s\n", node->name, idx, val_reg);
            free(val_reg);
        }
    }
}

static char* gen_function_call_expr(ASTNode* node) {
    // Generate param assignments with numbering
    int paramIndex = 0;
    ASTNode* arg = node->arguments;
    while (arg) {
        char* arg_reg = gen_expression(arg);
        // Assign the argument to a paramN temp before the PARAM line
        char paramVar[16];
        sprintf(paramVar, "param%d", paramIndex);
        fprintf(tac_out, "%s = %s\n", paramVar, arg_reg);
        fprintf(tac_out, "PARAM %s\n", paramVar);
        free(arg_reg);
        arg = arg->next;
        paramIndex++;
    }

    char* call_reg = new_temp();
    fprintf(tac_out, "%s = CALL %s\n", call_reg, node->string);
    return call_reg;
}

static char* gen_expression(ASTNode* expr) {
    if (!expr) {
        char* t = new_temp();
        fprintf(tac_out, "%s = 0\n", t);
        return t;
    }

    switch (expr->type) {
        case AST_EXPRESSION:
            if (expr->operator) {
                if (strcmp(expr->operator, "+") == 0 &&
                    expr->left && expr->left->type == AST_EXPRESSION &&
                    expr->left->operator && strcmp(expr->left->operator, "ID") == 0 &&
                    expr->right && expr->right->type == AST_EXPRESSION &&
                    expr->right->operator && strcmp(expr->right->operator, "NUMBER") == 0) {
                    return gen_increment_expr(expr->left->string, expr->right->value);
                }
                if (strcmp(expr->operator, "NUMBER") == 0) {
                    char* t = new_temp();
                    fprintf(tac_out, "%s = %d\n", t, expr->value);
                    return t;
                } else if (strcmp(expr->operator, "FLOAT_NUMBER") == 0) {
                    char* t = new_temp();
                    fprintf(tac_out, "%s = %.2f\n", t, expr->float_value);
                    return t;
                } else if (strcmp(expr->operator, "ID") == 0) {
                    char* t = new_temp();
                    fprintf(tac_out, "%s = %s\n", t, expr->string);
                    return t;
                } else if (strcmp(expr->operator, "!") == 0) {
                    char* operand = gen_expression(expr->left);
                    char* t = new_temp();
                    fprintf(tac_out, "%s = 1 - %s\n", t, operand);
                    free(operand);
                    return t;
                } else {
                    char* left_t = gen_expression(expr->left);
                    char* right_t = gen_expression(expr->right);
                    char* t = new_temp();
                    fprintf(tac_out, "%s = %s %s %s\n", t, left_t, expr->operator, right_t);
                    free(left_t);
                    free(right_t);
                    return t;
                }
            }
            break;
        case AST_ARRAY_ACCESS: {
            char* idx = gen_expression(expr->left);
            char* t = new_temp();
            fprintf(tac_out, "%s = %s[%s]\n", t, expr->string, idx);
            free(idx);
            return t;
        }
        case AST_FUNCTION_CALL:
            return gen_function_call_expr(expr);
        default: {
            char* t = new_temp();
            fprintf(tac_out, "%s = 0\n", t);
            return t;
        }
    }
    char* t = new_temp();
    fprintf(tac_out, "%s = 0\n", t);
    return t;
}

static void gen_char_assignment(const char* var, char ch) {
    fprintf(tac_out, "%s = '%c'\n", var, ch);
}

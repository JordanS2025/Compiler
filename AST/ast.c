#include "ast.h"
#include <stdlib.h>
#include <string.h>

// Node creation functions

ASTNode* createProgram(ASTNode* varDeclList, ASTNode* stmtList) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Program node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_PROGRAM;
    node->data.program.varDeclList = varDeclList;
    node->data.program.stmtList = stmtList;
    return node;
}

ASTNode* createVarDeclList(ASTNode* varDeclList, ASTNode* varDecl) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for VarDeclList node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_VAR_DECL_LIST;
    node->data.varDeclList.varDeclList = varDeclList;
    node->data.varDeclList.varDecl = varDecl;
    return node;
}

ASTNode* createVarDecl(ASTNode* type, const char* id, ASTNode* expr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for VarDecl node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_VAR_DECL;
    node->data.varDecl.type = type;
    node->data.varDecl.id = strdup(id);
    node->data.varDecl.expr = expr;
    return node;
}

ASTNode* createType(const char* typeStr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Type node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_TYPE;
    node->data.typeStr = strdup(typeStr);
    return node;
}

ASTNode* createStmtList(ASTNode* stmtList, ASTNode* stmt) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for StmtList node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_STMT_LIST;
    node->data.stmtList.stmtList = stmtList;
    node->data.stmtList.stmt = stmt;
    return node;
}

ASTNode* createStmtAssign(const char* id, ASTNode* expr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Stmt Assign node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_STMT_ASSIGN;
    node->data.stmtAssign.id = strdup(id);
    node->data.stmtAssign.expr = expr;
    return node;
}

ASTNode* createStmtWrite(const char* id) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Stmt Write node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_STMT_WRITE;
    node->data.writeId = strdup(id);
    return node;
}

ASTNode* createExprBinary(const char* op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Expr Binary node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_EXPR_BINARY;
    node->data.exprBinary.op = strdup(op);
    node->data.exprBinary.left = left;
    node->data.exprBinary.right = right;
    return node;
}

ASTNode* createExprId(const char* id) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Expr ID node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_EXPR_ID;
    node->data.exprId = strdup(id);
    return node;
}

ASTNode* createExprNumber(int number) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate memory for Expr Number node");
        exit(EXIT_FAILURE);
    }
    node->type = AST_EXPR_NUMBER;
    node->data.exprNumber = number;
    return node;
}

// Helper function to print indentation
static void printIndent(int level) {
    for(int i = 0; i < level; i++) {
        printf("  ");
    }
}

// AST printing function
void printAST(ASTNode* node, int level) {
    if (!node) return;
    printIndent(level);
    switch(node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            printAST(node->data.program.varDeclList, level + 1);
            printAST(node->data.program.stmtList, level + 1);
            break;
        case AST_VAR_DECL_LIST:
            printf("VarDeclList\n");
            printAST(node->data.varDeclList.varDeclList, level + 1);
            printAST(node->data.varDeclList.varDecl, level + 1);
            break;
        case AST_VAR_DECL:
            printf("VarDecl\n");
            printAST(node->data.varDecl.type, level + 1);
            printIndent(level + 1);
            printf("ID: %s\n", node->data.varDecl.id);
            if (node->data.varDecl.expr) {
                printIndent(level + 1);
                printf("Initializer:\n");
                printAST(node->data.varDecl.expr, level + 2);
            }
            break;
        case AST_TYPE:
            printf("Type: %s\n", node->data.typeStr);
            break;
        case AST_STMT_LIST:
            printf("StmtList\n");
            printAST(node->data.stmtList.stmtList, level + 1);
            printAST(node->data.stmtList.stmt, level + 1);
            break;
        case AST_STMT_ASSIGN:
            printf("Stmt Assign: %s\n", node->data.stmtAssign.id);
            printIndent(level + 1);
            printf("Expression:\n");
            printAST(node->data.stmtAssign.expr, level + 2);
            break;
        case AST_STMT_WRITE:
            printf("Stmt Write: %s\n", node->data.writeId);
            break;
        case AST_EXPR_BINARY:
            printf("Expr Binary: %s\n", node->data.exprBinary.op);
            printAST(node->data.exprBinary.left, level + 1);
            printAST(node->data.exprBinary.right, level + 1);
            break;
        case AST_EXPR_ID:
            printf("Expr ID: %s\n", node->data.exprId);
            break;
        case AST_EXPR_NUMBER:
            printf("Expr Number: %d\n", node->data.exprNumber);
            break;
        default:
            printf("Unknown AST Node Type\n");
    }
}

// AST freeing function
// AST freeing function
void freeAST(ASTNode* node) {
    if (!node) return;
    switch(node->type) {
        case AST_PROGRAM:
            freeAST(node->data.program.varDeclList);
            freeAST(node->data.program.stmtList);
            break;
        case AST_VAR_DECL_LIST:
            freeAST(node->data.varDeclList.varDeclList);
            freeAST(node->data.varDeclList.varDecl);
            break;
        case AST_VAR_DECL:
            freeAST(node->data.varDecl.type);
            free(node->data.varDecl.id);
            freeAST(node->data.varDecl.expr);
            break;
        case AST_TYPE:
            free(node->data.typeStr);
            break;
        case AST_STMT_LIST:
            freeAST(node->data.stmtList.stmtList);
            freeAST(node->data.stmtList.stmt);
            break;
        case AST_STMT_ASSIGN:
            free(node->data.stmtAssign.id);
            freeAST(node->data.stmtAssign.expr);
            break;
        case AST_STMT_WRITE:
            free(node->data.writeId);
            break;
        case AST_EXPR_BINARY:
            free(node->data.exprBinary.op);
            freeAST(node->data.exprBinary.left);
            freeAST(node->data.exprBinary.right);
            break;
        case AST_EXPR_ID:
            free(node->data.exprId);
            break;
        case AST_EXPR_NUMBER:
            // Nothing to free for integer
            break;
        default:
            // Unknown node type
            break;
    }
    free(node);
}


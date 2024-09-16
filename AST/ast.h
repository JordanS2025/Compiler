#ifndef AST_H
#define AST_H

#include <stdio.h>

// Enumeration of AST node types
typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL_LIST,
    AST_VAR_DECL,
    AST_TYPE,
    AST_STMT_LIST,
    AST_STMT_ASSIGN,
    AST_STMT_WRITE,
    AST_EXPR_BINARY,
    AST_EXPR_ID,
    AST_EXPR_NUMBER
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// AST Node structure
struct ASTNode {
    ASTNodeType type;
    union {
        // For Program: VarDeclList and StmtList
        struct {
            ASTNode* varDeclList;
            ASTNode* stmtList;
        } program;

        // For VarDeclList: left and varDecl
        struct {
            ASTNode* varDeclList;
            ASTNode* varDecl;
        } varDeclList;

        // For VarDecl
        struct {
            ASTNode* type;
            char* id;
            ASTNode* expr; // NULL if no initialization
        } varDecl;

        // For Type
        char* typeStr;

        // For StmtList
        struct {
            ASTNode* stmtList;
            ASTNode* stmt;
        } stmtList;

        // For Stmt Assign
        struct {
            char* id;
            ASTNode* expr;
        } stmtAssign;

        // For Stmt Write
        char* writeId;

        // For Expr Binary
        struct {
            char* op;
            ASTNode* left;
            ASTNode* right;
        } exprBinary;

        // For Expr ID
        char* exprId;

        // For Expr Number
        int exprNumber;
    } data;
};

// Function declarations

// Node creation functions
ASTNode* createProgram(ASTNode* varDeclList, ASTNode* stmtList);
ASTNode* createVarDeclList(ASTNode* varDeclList, ASTNode* varDecl);
ASTNode* createVarDecl(ASTNode* type, const char* id, ASTNode* expr); // expr can be NULL
ASTNode* createType(const char* typeStr);
ASTNode* createStmtList(ASTNode* stmtList, ASTNode* stmt);
ASTNode* createStmtAssign(const char* id, ASTNode* expr);
ASTNode* createStmtWrite(const char* id);
ASTNode* createExprBinary(const char* op, ASTNode* left, ASTNode* right);
ASTNode* createExprId(const char* id);
ASTNode* createExprNumber(int number);

// AST printing function
void printAST(ASTNode* node, int level);

// AST freeing function
void freeAST(ASTNode* node);

#endif // AST_H


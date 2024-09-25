// semantic.h
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symboltable.h"

// Enumeration of TAC operation codes
typedef enum {
    TAC_ASSIGN,        // x = 5
    TAC_ADD,           // x = y + z
    TAC_WRITE,         // write x
    // Add other TAC operations as needed
} TACOp;

// Structure for a TAC instruction
typedef struct TAC {
    TACOp op;                // Operation code
    char* arg1;              // First argument
    char* arg2;              // Second argument
    char* result;            // Result
    struct TAC* next;        // Pointer to the next TAC instruction
} TAC;

// Structure for a list of TAC instructions
typedef struct {
    TAC* head;               // Head of the TAC list
    TAC* tail;               // Tail of the TAC list
} TACList;

// Structure for managing temporary variables
typedef struct TempVar {
    int count;
} TempVar;

// Function Prototypes

// Initialize the TAC list
void initTACList(TACList* list);

// Create a new TAC instruction
TAC* createTAC(TACOp op, const char* arg1, const char* arg2, const char* result);

// Add a TAC instruction to the list
void addTAC(TACList* list, TAC* tac);

// Generate a new temporary variable name
char* newTemp(TempVar* temp);

// Free the TAC list
void freeTACList(TACList* list);

// Print the TAC list
void printTACList(const TACList* list);

// Semantic analysis and TAC generation
TACList* performSemanticAnalysis(ASTNode* ast, SymbolTable* symtab);

// Helper functions for traversing the AST and generating TAC
TACList* generateProgram(ASTNode* node, SymbolTable* symtab, TempVar* temp);
TACList* generateVarDeclList(ASTNode* node, SymbolTable* symtab, TempVar* temp);
TACList* generateVarDecl(ASTNode* node, SymbolTable* symtab, TempVar* temp);
TACList* generateStmtList(ASTNode* node, SymbolTable* symtab, TempVar* temp);
TACList* generateStmt(ASTNode* node, SymbolTable* symtab, TempVar* temp);
char* generateExpr(ASTNode* node, SymbolTable* symtab, TACList* tacList, TempVar* temp, const char* target);

// Error handling during semantic analysis
void semanticError(const char* message, int line);

#endif // SEMANTIC_H

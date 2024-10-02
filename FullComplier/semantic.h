#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symboltable.h"

// Enumeration of TAC operation types
typedef enum {
    TAC_ASSIGN,    // t = a
    TAC_ADD,       // t = a + b
    TAC_SUB,       // t = a - b
    TAC_MUL,       // t = a * b
    TAC_DIV,       // t = a / b
    TAC_WRITE      // write t
    // Add more operations as needed
} TACOp;

// Structure for a TAC instruction
typedef struct TACInstruction {
    TACOp op;                     // Operation type
    char* result;                 // Result variable (e.g., t0)
    char* arg1;                   // First argument (e.g., t1 or constant)
    char* arg2;                   // Second argument (e.g., t2 or constant), NULL if not applicable
    struct TACInstruction* next;  // Pointer to the next instruction
} TACInstruction;

// Structure for the TAC list
typedef struct {
    TACInstruction* head; // Pointer to the first instruction
    TACInstruction* tail; // Pointer to the last instruction
} TACList;

// Function prototypes for TAC management
TACList* createTACList();
void appendTAC(TACList* list, TACOp op, const char* result, const char* arg1, const char* arg2);
void printTACList(const TACList* list);
void freeTACList(TACList* list);

// Declare tacList as an external variable
extern TACList* tacList;

// Function prototypes for Semantic Analysis
void initializeSemantic(SymbolTable* symbolTable);
void generateCode(ASTNode* node, SymbolTable* symbolTable);
void finalizeSemantic();

#endif // SEMANTIC_H

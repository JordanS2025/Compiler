#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define tacList as a global variable
TACList* tacList = NULL;

// Counter for temporary variables
static int tempCount = 0;

// Mapping from variable names to their corresponding temporaries
typedef struct VarTempMap {
    char* varName;
    char* tempName;
    struct VarTempMap* next;
} VarTempMap;

static VarTempMap* varTempMapHead = NULL;

// Initialize semantic analysis
void initializeSemantic(SymbolTable* symbolTable) {
    tempCount = 0;
    varTempMapHead = NULL;
    tacList = createTACList(); // Initialize TAC list
}

// Function to create a TAC list
TACList* createTACList() {
    TACList* list = (TACList*)malloc(sizeof(TACList));
    if (!list) {
        perror("Failed to allocate memory for TACList");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->tail = NULL;
    return list;
}

// Function to append a TAC instruction to the list
void appendTAC(TACList* list, TACOp op, const char* result, const char* arg1, const char* arg2) {
    TACInstruction* instr = (TACInstruction*)malloc(sizeof(TACInstruction));
    if (!instr) {
        perror("Failed to allocate memory for TACInstruction");
        exit(EXIT_FAILURE);
    }
    instr->op = op;
    instr->result = result ? strdup(result) : NULL;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    instr->next = NULL;

    if (list->tail) {
        list->tail->next = instr;
        list->tail = instr;
    } else {
        list->head = instr;
        list->tail = instr;
    }
}

// Function to print the TAC list
void printTACList(const TACList* list) {
    TACInstruction* current = list->head;
    while (current) {
        switch (current->op) {
            case TAC_ASSIGN:
                printf("%s = %s\n", current->result, current->arg1);
                break;
            case TAC_ADD:
                printf("%s = %s + %s\n", current->result, current->arg1, current->arg2);
                break;
            case TAC_SUB:
                printf("%s = %s - %s\n", current->result, current->arg1, current->arg2);
                break;
            case TAC_MUL:
                printf("%s = %s * %s\n", current->result, current->arg1, current->arg2);
                break;
            case TAC_DIV:
                printf("%s = %s / %s\n", current->result, current->arg1, current->arg2);
                break;
            case TAC_WRITE:
                printf("write %s\n", current->arg1);
                break;
            default:
                printf("Unknown TAC operation\n");
        }
        current = current->next;
    }
}

// Function to free the TAC list
void freeTACList(TACList* list) {
    TACInstruction* current = list->head;
    while (current) {
        TACInstruction* temp = current;
        current = current->next;
        if (temp->result) free(temp->result);
        if (temp->arg1) free(temp->arg1);
        if (temp->arg2) free(temp->arg2);
        free(temp);
    }
    free(list);
}

// Generate a new temporary variable
char* newTemp() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "t%d", tempCount++);
    return temp;
}

// Map a variable name to a temporary variable
void addVarTempMapping(const char* varName, const char* tempName) {
    VarTempMap* newMapping = (VarTempMap*)malloc(sizeof(VarTempMap));
    newMapping->varName = strdup(varName);
    newMapping->tempName = strdup(tempName);
    newMapping->next = varTempMapHead;
    varTempMapHead = newMapping;
}

// Get the temporary variable for a given variable name
char* getTempForVar(const char* varName) {
    VarTempMap* current = varTempMapHead;
    while (current != NULL) {
        if (strcmp(current->varName, varName) == 0) {
            return current->tempName;
        }
        current = current->next;
    }
    return NULL;
}

// Free the variable-to-temporary mappings
void freeVarTempMap() {
    VarTempMap* current = varTempMapHead;
    while (current != NULL) {
        VarTempMap* temp = current;
        current = current->next;
        free(temp->varName);
        free(temp->tempName);
        free(temp);
    }
    varTempMapHead = NULL;
}

// Recursive function to generate code for expressions
char* generateExprCode(ASTNode* node, SymbolTable* symbolTable) {
    if (!node) return NULL;

    switch (node->type) {
        case AST_EXPR_NUMBER: {
            char* constStr = (char*)malloc(20);
            sprintf(constStr, "%d", node->data.exprNumber);
            return constStr;
        }

        case AST_EXPR_ID: {
            char* tempVar = getTempForVar(node->data.exprId);
            if (!tempVar) {
                fprintf(stderr, "Error: Undeclared variable '%s'\n", node->data.exprId);
                exit(EXIT_FAILURE);
            }
            return strdup(tempVar);
        }

        case AST_EXPR_BINARY: {
            char* left = generateExprCode(node->data.exprBinary.left, symbolTable);
            char* right = generateExprCode(node->data.exprBinary.right, symbolTable);
            char* resultTemp = newTemp();

            // Determine the operation
            TACOp op;
            if (strcmp(node->data.exprBinary.op, "+") == 0) {
                op = TAC_ADD;
            } else if (strcmp(node->data.exprBinary.op, "-") == 0) {
                op = TAC_SUB;
            } else if (strcmp(node->data.exprBinary.op, "*") == 0) {
                op = TAC_MUL;
            } else if (strcmp(node->data.exprBinary.op, "/") == 0) {
                op = TAC_DIV;
            } else {
                fprintf(stderr, "Error: Unsupported binary operator '%s'\n", node->data.exprBinary.op);
                exit(EXIT_FAILURE);
            }

            appendTAC(tacList, op, resultTemp, left, right);
            free(left);
            free(right);

            return resultTemp;
        }

        default:
            fprintf(stderr, "Error: Unsupported AST node in generateExprCode\n");
            exit(EXIT_FAILURE);
    }
}

// Generate TAC code from the AST
void generateCode(ASTNode* node, SymbolTable* symbolTable) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            generateCode(node->data.program.varDeclList, symbolTable);
            generateCode(node->data.program.stmtList, symbolTable);
            break;

        case AST_VAR_DECL_LIST:
            generateCode(node->data.varDeclList.varDeclList, symbolTable);
            generateCode(node->data.varDeclList.varDecl, symbolTable);
            break;

        case AST_VAR_DECL: {
            const char* varName = node->data.varDecl.id;
            const char* varType = node->data.varDecl.type->data.typeStr;
            addSymbol(symbolTable, varName, varType, 0);

            char* temp = newTemp();
            addVarTempMapping(varName, temp);

            if (node->data.varDecl.expr != NULL) {
                char* exprTemp = generateExprCode(node->data.varDecl.expr, symbolTable);
                appendTAC(tacList, TAC_ASSIGN, temp, exprTemp, NULL);
                free(exprTemp);
            }
            break;
        }

        case AST_STMT_LIST:
            generateCode(node->data.stmtList.stmtList, symbolTable);
            generateCode(node->data.stmtList.stmt, symbolTable);
            break;

        case AST_STMT_ASSIGN: {
            const char* varName = node->data.stmtAssign.id;
            char* tempVar = getTempForVar(varName);
            if (!tempVar) {
                fprintf(stderr, "Error: Undeclared variable '%s'\n", varName);
                exit(EXIT_FAILURE);
            }

            char* exprTemp = generateExprCode(node->data.stmtAssign.expr, symbolTable);
            appendTAC(tacList, TAC_ASSIGN, tempVar, exprTemp, NULL);
            free(exprTemp);
            break;
        }

        case AST_STMT_WRITE: {
            const char* varName = node->data.writeId;
            char* tempVar = getTempForVar(varName);
            if (!tempVar) {
                fprintf(stderr, "Error: Undeclared variable '%s'\n", varName);
                exit(EXIT_FAILURE);
            }
            appendTAC(tacList, TAC_WRITE, NULL, tempVar, NULL);
            break;
        }

        default:
            fprintf(stderr, "Error: Unsupported AST node in generateCode\n");
            exit(EXIT_FAILURE);
    }
}

// Finalize semantic analysis
void finalizeSemantic() {
    freeVarTempMap();
}

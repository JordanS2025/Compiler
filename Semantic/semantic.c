// semantic.c
#include "semantic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initialize the TAC list
void initTACList(TACList* list) {
    list->head = NULL;
    list->tail = NULL;
}

// Create a new TAC instruction
TAC* createTAC(TACOp op, const char* arg1, const char* arg2, const char* result) {
    TAC* tac = (TAC*)malloc(sizeof(TAC));
    tac->op = op;
    tac->arg1 = arg1 ? strdup(arg1) : NULL;
    tac->arg2 = arg2 ? strdup(arg2) : NULL;
    tac->result = result ? strdup(result) : NULL;
    tac->next = NULL;
    return tac;
}

// Add a TAC instruction to the list
void addTAC(TACList* list, TAC* tac) {
    if (list->head == NULL) {
        list->head = tac;
        list->tail = tac;
    } else {
        list->tail->next = tac;
        list->tail = tac;
    }
}

// Generate a new temporary variable name
char* newTemp(TempVar* temp) {
    char buffer[20];
    sprintf(buffer, "t%d", temp->count++);
    return strdup(buffer);
}

// Free the TAC list
void freeTACList(TACList* list) {
    TAC* current = list->head;
    while (current) {
        TAC* temp = current;
        current = current->next;
        free(temp->arg1);
        free(temp->arg2);
        free(temp->result);
        free(temp);
    }
    list->head = NULL;
    list->tail = NULL;
}

// Print the TAC list
void printTACList(const TACList* list) {
    TAC* current = list->head;
    while (current) {
        switch (current->op) {
            case TAC_ASSIGN:
                printf("%s = %s\n", current->result, current->arg1);
                break;
            case TAC_ADD:
                printf("%s = %s + %s\n", current->result, current->arg1, current->arg2);
                break;
            case TAC_WRITE:
                printf("print %s\n", current->arg1);
                break;
            // Add cases for other TAC operations as needed
            default:
                printf("Unknown TAC operation\n");
        }
        current = current->next;
    }
}

// Semantic error handling
void semanticError(const char* message, int line) {
    fprintf(stderr, "Semantic Error (Line %d): %s\n", line, message);
    exit(EXIT_FAILURE);
}

// Perform semantic analysis and generate TAC
TACList* performSemanticAnalysis(ASTNode* ast, SymbolTable* symtab) {
    TempVar temp;
    temp.count = 1;
    TACList* tacList = (TACList*)malloc(sizeof(TACList));
    initTACList(tacList);
    
    TACList* programTAC = generateProgram(ast, symtab, &temp);
    
    // Append programTAC to tacList
    if (programTAC->head) {
        tacList->head = programTAC->head;
        tacList->tail = programTAC->tail;
    }
    free(programTAC);
    
    return tacList;
}

// Generate TAC for the Program node
TACList* generateProgram(ASTNode* node, SymbolTable* symtab, TempVar* temp) {
    TACList* list = (TACList*)malloc(sizeof(TACList));
    initTACList(list);
    
    // Assuming Program node has varDeclList and stmtList
    if (node->type != AST_PROGRAM) {
        semanticError("Expected Program node", 0);
    }
    
    if (node->data.program.varDeclList) {
        TACList* varTAC = generateVarDeclList(node->data.program.varDeclList, symtab, temp);
        // Append varTAC to list
        if (varTAC->head) {
            list->head = varTAC->head;
            list->tail = varTAC->tail;
        }
        free(varTAC);
    }
    
    if (node->data.program.stmtList) {
        TACList* stmtTAC = generateStmtList(node->data.program.stmtList, symtab, temp);
        // Append stmtTAC to list
        if (stmtTAC->head) {
            if (list->head == NULL) {
                list->head = stmtTAC->head;
                list->tail = stmtTAC->tail;
            } else {
                list->tail->next = stmtTAC->head;
                list->tail = stmtTAC->tail;
            }
        }
        free(stmtTAC);
    }
    
    return list;
}

// Generate TAC for VarDeclList
TACList* generateVarDeclList(ASTNode* node, SymbolTable* symtab, TempVar* temp) {
    TACList* list = (TACList*)malloc(sizeof(TACList));
    initTACList(list);
    
    if (node == NULL) {
        return list;
    }
    
    // Recursively generate TAC for VarDeclList
    if (node->data.varDeclList.varDeclList) {
        TACList* leftTAC = generateVarDeclList(node->data.varDeclList.varDeclList, symtab, temp);
        // Append leftTAC to list
        if (leftTAC->head) {
            list->head = leftTAC->head;
            list->tail = leftTAC->tail;
        }
        free(leftTAC);
    }
    
    // Generate TAC for the current VarDecl
    TACList* varTAC = generateVarDecl(node->data.varDeclList.varDecl, symtab, temp);
    if (varTAC->head) {
        if (list->head == NULL) {
            list->head = varTAC->head;
            list->tail = varTAC->tail;
        } else {
            list->tail->next = varTAC->head;
            list->tail = varTAC->tail;
        }
    }
    free(varTAC);
    
    return list;
}

// Generate TAC for a single VarDecl
TACList* generateVarDecl(ASTNode* node, SymbolTable* symtab, TempVar* temp) {
    TACList* list = (TACList*)malloc(sizeof(TACList));
    initTACList(list);
    
    if (node->type != AST_VAR_DECL) {
        semanticError("Expected VarDecl node", 0);
    }
    
    char* varName = node->data.varDecl.id;
    
    if (node->data.varDecl.expr) {
        // Variable initialization: x = expr
        char* exprResult = generateExpr(node->data.varDecl.expr, symtab, list, temp, varName);
        // Since exprResult is varName, no additional assignment is needed
        // Example: x = 5 + y
        // No need for temp variable
        free(exprResult);
    }
    // If no initialization, no TAC needed for declaration
    
    return list;
}

// Generate TAC for StmtList
TACList* generateStmtList(ASTNode* node, SymbolTable* symtab, TempVar* temp) {
    TACList* list = (TACList*)malloc(sizeof(TACList));
    initTACList(list);
    
    if (node == NULL) {
        return list;
    }
    
    // Recursively generate TAC for StmtList
    if (node->data.stmtList.stmtList) {
        TACList* leftTAC = generateStmtList(node->data.stmtList.stmtList, symtab, temp);
        // Append leftTAC to list
        if (leftTAC->head) {
            list->head = leftTAC->head;
            list->tail = leftTAC->tail;
        }
        free(leftTAC);
    }
    
    // Generate TAC for the current Stmt
    TACList* stmtTAC = generateStmt(node->data.stmtList.stmt, symtab, temp);
    if (stmtTAC->head) {
        if (list->head == NULL) {
            list->head = stmtTAC->head;
            list->tail = stmtTAC->tail;
        } else {
            list->tail->next = stmtTAC->head;
            list->tail = stmtTAC->tail;
        }
    }
    free(stmtTAC);
    
    return list;
}

// Generate TAC for a single Stmt
TACList* generateStmt(ASTNode* node, SymbolTable* symtab, TempVar* temp) {
    TACList* list = (TACList*)malloc(sizeof(TACList));
    initTACList(list);
    
    if (node->type == AST_STMT_ASSIGN) {
        char* varName = node->data.stmtAssign.id;
        // Pass varName as the target to generateExpr
        char* exprResult = generateExpr(node->data.stmtAssign.expr, symtab, list, temp, varName);
        // Since exprResult is varName, no need for an additional assignment
        free(exprResult);
    }
    else if (node->type == AST_STMT_WRITE) {
        char* varName = node->data.writeId;
        TAC* printTAC = createTAC(TAC_WRITE, varName, NULL, NULL);
        addTAC(list, printTAC);
    }
    else {
        semanticError("Unknown statement type", 0);
    }
    
    return list;
}

// Generate TAC for an expression and return the result variable name
// If 'target' is not NULL, store the result directly in 'target'
char* generateExpr(ASTNode* node, SymbolTable* symtab, TACList* tacList, TempVar* temp, const char* target) {
    if (node->type == AST_EXPR_NUMBER) {
        char buffer[20];
        sprintf(buffer, "%d", node->data.exprNumber);
        if (target) {
            // Direct assignment to target
            TAC* assignTAC = createTAC(TAC_ASSIGN, buffer, NULL, target);
            addTAC(tacList, assignTAC);
            return strdup(target);
        }
        return strdup(buffer);
    }
    else if (node->type == AST_EXPR_ID) {
        if (target) {
            // Direct assignment to target
            TAC* assignTAC = createTAC(TAC_ASSIGN, node->data.exprId, NULL, target);
            addTAC(tacList, assignTAC);
            return strdup(target);
        }
        return strdup(node->data.exprId);
    }
    else if (node->type == AST_EXPR_BINARY) {
        char* left = generateExpr(node->data.exprBinary.left, symtab, tacList, temp, NULL);
        char* right = generateExpr(node->data.exprBinary.right, symtab, tacList, temp, NULL);
        
        char* resultVar = target ? strdup(target) : newTemp(temp);
        
        TACOp op;
        if (strcmp(node->data.exprBinary.op, "+") == 0) {
            op = TAC_ADD;
        }
        else {
            semanticError("Unknown binary operator", 0);
        }
        
        TAC* binaryTAC = createTAC(op, left, right, resultVar);
        addTAC(tacList, binaryTAC);
        
        free(left);
        free(right);
        
        return resultVar;
    }
    else {
        semanticError("Unknown expression type", 0);
        return NULL;
    }
}

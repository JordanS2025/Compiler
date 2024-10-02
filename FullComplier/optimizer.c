// optimizer.c
#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mapping for constant propagation
typedef struct ConstMapEntry {
    char* var;
    int value;
    struct ConstMapEntry* next;
} ConstMapEntry;

// Mapping from value to temporary variables
typedef struct ValueMapEntry {
    int value;
    char* temp;
    struct ValueMapEntry* next;
} ValueMapEntry;

// Head of the constant map linked list
static ConstMapEntry* constMapHead = NULL;

// Head of the value map linked list
static ValueMapEntry* valueMapHead = NULL;

// Function to add/update a constant mapping
void addConstMapping(const char* var, int value) {
    ConstMapEntry* current = constMapHead;
    while (current != NULL) {
        if (strcmp(current->var, var) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Not found, add new entry
    ConstMapEntry* newEntry = (ConstMapEntry*)malloc(sizeof(ConstMapEntry));
    newEntry->var = strdup(var);
    newEntry->value = value;
    newEntry->next = constMapHead;
    constMapHead = newEntry;
}

// Function to remove a variable from the constant map
void removeConstMapping(const char* var) {
    ConstMapEntry* current = constMapHead;
    ConstMapEntry* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->var, var) == 0) {
            if (prev == NULL) {
                constMapHead = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->var);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Function to get the constant value of a variable, returns 1 if found, 0 otherwise
int getConstValue(const char* var, int* value) {
    ConstMapEntry* current = constMapHead;
    while (current != NULL) {
        if (strcmp(current->var, var) == 0) {
            *value = current->value;
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Function to add a value-to-temp mapping
void addValueMapping(int value, const char* temp) {
    ValueMapEntry* current = valueMapHead;
    while (current != NULL) {
        if (current->value == value) {
            // Value already mapped
            return;
        }
        current = current->next;
    }

    // Not found, add new entry
    ValueMapEntry* newEntry = (ValueMapEntry*)malloc(sizeof(ValueMapEntry));
    newEntry->value = value;
    newEntry->temp = strdup(temp);
    newEntry->next = valueMapHead;
    valueMapHead = newEntry;
}

// Function to get the temporary for a given value, returns NULL if not found
char* getTempForValue(int value) {
    ValueMapEntry* current = valueMapHead;
    while (current != NULL) {
        if (current->value == value) {
            return current->temp;
        }
        current = current->next;
    }
    return NULL;
}

// Function to remove a value-to-temp mapping
void removeValueMapping(int value) {
    ValueMapEntry* current = valueMapHead;
    ValueMapEntry* prev = NULL;

    while (current != NULL) {
        if (current->value == value) {
            if (prev == NULL) {
                valueMapHead = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->temp);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Function to replace all uses of oldTemp with newTemp in the TAC list
void replaceTempInTAC(TACList* list, const char* oldTemp, const char* newTemp) {
    TACInstruction* current = list->head;
    while (current != NULL) {
        if (current->arg1 && strcmp(current->arg1, oldTemp) == 0) {
            free(current->arg1);
            current->arg1 = strdup(newTemp);
        }
        if (current->arg2 && strcmp(current->arg2, oldTemp) == 0) {
            free(current->arg2);
            current->arg2 = strdup(newTemp);
        }
        current = current->next;
    }
}

// Function to count the usage of each temporary
typedef struct UsageCount {
    char* temp;
    int count;
    struct UsageCount* next;
} UsageCount;

static UsageCount* usageHead = NULL;

// Function to initialize usage counts
void initializeUsageCounts(TACList* list) {
    usageHead = NULL;
    TACInstruction* current = list->head;
    while (current != NULL) {
        if (current->arg1 && current->arg1[0] == 't') {
            UsageCount* uc = usageHead;
            while (uc != NULL) {
                if (strcmp(uc->temp, current->arg1) == 0) {
                    uc->count++;
                    break;
                }
                uc = uc->next;
            }
            if (uc == NULL) {
                UsageCount* newUC = (UsageCount*)malloc(sizeof(UsageCount));
                newUC->temp = strdup(current->arg1);
                newUC->count = 1;
                newUC->next = usageHead;
                usageHead = newUC;
            }
        }
        if (current->arg2 && current->arg2[0] == 't') {
            UsageCount* uc = usageHead;
            while (uc != NULL) {
                if (strcmp(uc->temp, current->arg2) == 0) {
                    uc->count++;
                    break;
                }
                uc = uc->next;
            }
            if (uc == NULL) {
                UsageCount* newUC = (UsageCount*)malloc(sizeof(UsageCount));
                newUC->temp = strdup(current->arg2);
                newUC->count = 1;
                newUC->next = usageHead;
                usageHead = newUC;
            }
        }
        current = current->next;
    }
}

// Function to get usage count for a temporary
int getUsageCount(const char* temp) {
    UsageCount* current = usageHead;
    while (current != NULL) {
        if (strcmp(current->temp, temp) == 0) {
            return current->count;
        }
        current = current->next;
    }
    return 0;
}

// Function to free usage counts
void freeUsageCounts() {
    UsageCount* current = usageHead;
    while (current != NULL) {
        UsageCount* temp = current;
        current = current->next;
        free(temp->temp);
        free(temp);
    }
    usageHead = NULL;
}

// Function to perform Dead Code Elimination and Unused Temporary Removal
void eliminateDeadCode(TACList* list) {
    initializeUsageCounts(list);
    TACInstruction* current = list->head;
    TACInstruction* prev = NULL;

    while (current != NULL) {
        TACInstruction* nextInstr = current->next;

        // Check if the result is a temporary
        if (current->result && current->result[0] == 't') {
            // Get usage count
            int usage = getUsageCount(current->result);
            if (usage == 0) {
                // This instruction's result is never used; remove it
                if (prev == NULL) {
                    list->head = current->next;
                } else {
                    prev->next = current->next;
                }
                if (current == list->tail) {
                    list->tail = prev;
                }
                free(current->result);
                if (current->arg1) free(current->arg1);
                if (current->arg2) free(current->arg2);
                free(current);
                current = nextInstr;
                continue;
            }
        }

        prev = current;
        current = nextInstr;
    }

    freeUsageCounts();
}

// Initialize the optimizer (currently does nothing)
void initializeOptimizer() {
    // No initialization needed currently
}

// Perform constant folding, constant propagation, dead code elimination, and unused temporary removal on the TAC list
void optimizeTAC() {
    if (tacList == NULL) return;

    // Step 1: Constant Folding and Propagation
    TACInstruction* current = tacList->head;
    TACInstruction* prev = NULL;

    while (current != NULL) {
        TACInstruction* nextInstr = current->next;

        switch (current->op) {
            case TAC_ASSIGN: {
                // Check if arg1 is a constant
                int constValue;
                if (sscanf(current->arg1, "%d", &constValue) == 1) {
                    // arg1 is a constant, map result to constant
                    char* existingTemp = getTempForValue(constValue);
                    if (existingTemp != NULL) {
                        // Replace all uses of current->result with existingTemp
                        replaceTempInTAC(tacList, current->result, existingTemp);

                        // Remove the current instruction as it's redundant
                        if (prev == NULL) {
                            tacList->head = current->next;
                        } else {
                            prev->next = current->next;
                        }
                        if (current == tacList->tail) {
                            tacList->tail = prev;
                        }
                        free(current->result);
                        free(current->arg1);
                        free(current);
                        current = nextInstr;
                        continue;
                    } else {
                        // No existing temp for this value, add to value map
                        addValueMapping(constValue, current->result);
                    }

                    addConstMapping(current->result, constValue);
                } else {
                    // arg1 is a variable, check if it's a constant
                    if (getConstValue(current->arg1, &constValue)) {
                        // Replace arg1 with the constant value
                        free(current->arg1);
                        current->arg1 = (char*)malloc(20);
                        sprintf(current->arg1, "%d", constValue);

                        // Check if this constant is already mapped to a temp
                        char* existingTemp = getTempForValue(constValue);
                        if (existingTemp != NULL) {
                            // Replace all uses of current->result with existingTemp
                            replaceTempInTAC(tacList, current->result, existingTemp);

                            // Remove the current instruction as it's redundant
                            if (prev == NULL) {
                                tacList->head = current->next;
                            } else {
                                prev->next = current->next;
                            }
                            if (current == tacList->tail) {
                                tacList->tail = prev;
                            }
                            free(current->result);
                            free(current->arg1);
                            free(current);
                            current = nextInstr;
                            continue;
                        } else {
                            // No existing temp for this value, add to value map
                            addValueMapping(constValue, current->result);
                        }

                        addConstMapping(current->result, constValue);
                    } else {
                        // arg1 is not a constant, remove any existing mapping
                        removeConstMapping(current->result);
                        // No action needed for value map in this case
                    }
                }
                break;
            }

            case TAC_ADD:
            case TAC_SUB:
            case TAC_MUL:
            case TAC_DIV: {
                int leftConst = 0, rightConst = 0;
                int leftVal = 0, rightVal = 0;

                // Check if arg1 is a constant
                if (sscanf(current->arg1, "%d", &leftVal) == 1) {
                    leftConst = 1;
                } else if (getConstValue(current->arg1, &leftVal)) {
                    leftConst = 1;
                }

                // Check if arg2 is a constant
                if (sscanf(current->arg2, "%d", &rightVal) == 1) {
                    rightConst = 1;
                } else if (getConstValue(current->arg2, &rightVal)) {
                    rightConst = 1;
                }

                if (leftConst && rightConst) {
                    // Perform constant folding
                    int resultVal = 0;
                    switch (current->op) {
                        case TAC_ADD:
                            resultVal = leftVal + rightVal;
                            break;
                        case TAC_SUB:
                            resultVal = leftVal - rightVal;
                            break;
                        case TAC_MUL:
                            resultVal = leftVal * rightVal;
                            break;
                        case TAC_DIV:
                            if (rightVal != 0) {
                                resultVal = leftVal / rightVal;
                            } else {
                                fprintf(stderr, "Error: Division by zero in optimization.\n");
                                exit(EXIT_FAILURE);
                            }
                            break;
                        default:
                            break;
                    }

                    // Check if this constant is already mapped to a temp
                    char* existingTemp = getTempForValue(resultVal);
                    if (existingTemp != NULL) {
                        // Replace all uses of current->result with existingTemp
                        replaceTempInTAC(tacList, current->result, existingTemp);

                        // Remove the current instruction as it's redundant
                        if (prev == NULL) {
                            tacList->head = current->next;
                        } else {
                            prev->next = current->next;
                        }
                        if (current == tacList->tail) {
                            tacList->tail = prev;
                        }
                        free(current->result);
                        free(current->arg1);
                        if (current->arg2) free(current->arg2);
                        free(current);
                        current = nextInstr;
                        continue;
                    } else {
                        // No existing temp for this value, add to value map
                        addValueMapping(resultVal, current->result);
                    }

                    // Replace the current instruction with an assignment of the constant
                    current->op = TAC_ASSIGN;
                    free(current->arg2);
                    free(current->arg1);
                    current->arg1 = (char*)malloc(20);
                    sprintf(current->arg1, "%d", resultVal);
                    current->arg2 = NULL;

                    // Map result to the constant
                    addConstMapping(current->result, resultVal);
                } else {
                    // Propagate constants if possible
                    if (leftConst) {
                        free(current->arg1);
                        current->arg1 = (char*)malloc(20);
                        sprintf(current->arg1, "%d", leftVal);
                    }
                    if (rightConst) {
                        free(current->arg2);
                        current->arg2 = (char*)malloc(20);
                        sprintf(current->arg2, "%d", rightVal);
                    }

                    // If the operation result is not a constant, remove any existing mapping
                    removeConstMapping(current->result);
                }

                break;
            }

            case TAC_WRITE: {
                // No optimization needed for write
                break;
            }

            default:
                // Unsupported operation
                break;
        }

        prev = current;
        current = nextInstr;
    }

    // Step 2: Dead Code Elimination (Remove assignments to unused temporaries)
    eliminateDeadCode(tacList);

    // Step 3: Replace temporaries that hold constants and are used only once with the constants directly
    // Re-initialize usage counts after DCE
    initializeUsageCounts(tacList);
    current = tacList->head;
    prev = NULL;

    while (current != NULL) {
        TACInstruction* nextInstr = current->next;

        // Check if the instruction assigns a constant to a temporary
        if (current->op == TAC_ASSIGN && current->result && current->result[0] == 't') {
            int constValue;
            if (sscanf(current->arg1, "%d", &constValue) == 1) {
                // Check usage count
                int usage = getUsageCount(current->result);
                if (usage <= 1) { // Used zero or one time
                    // Find the instruction where it's used
                    // If used once, replace the use with the constant
                    if (usage == 1) {
                        replaceTempInTAC(tacList, current->result, current->arg1);
                    }

                    // Remove the current instruction
                    if (prev == NULL) {
                        tacList->head = current->next;
                    } else {
                        prev->next = current->next;
                    }
                    if (current == tacList->tail) {
                        tacList->tail = prev;
                    }
                    free(current->result);
                    free(current->arg1);
                    if (current->arg2) free(current->arg2);
                    free(current);
                    current = nextInstr;
                    continue;
                }
            }
        }

        prev = current;
        current = nextInstr;
    }

    freeUsageCounts();

    // Step 4: Final Dead Code Elimination after replacements
    eliminateDeadCode(tacList);
}

// Print the optimized TAC
void printOptimizedTAC(const TACList* list) {
    printTACList(list);
}

// Finalize the optimizer (frees the constant and value maps)
void finalizeOptimizer() {
    // Free the constant map
    ConstMapEntry* currentConst = constMapHead;
    while (currentConst != NULL) {
        ConstMapEntry* temp = currentConst;
        currentConst = currentConst->next;
        free(temp->var);
        free(temp);
    }
    constMapHead = NULL;

    // Free the value map
    ValueMapEntry* currentValue = valueMapHead;
    while (currentValue != NULL) {
        ValueMapEntry* temp = currentValue;
        currentValue = currentValue->next;
        free(temp->temp);
        free(temp);
    }
    valueMapHead = NULL;
}

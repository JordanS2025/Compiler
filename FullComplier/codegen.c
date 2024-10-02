// codegen.c
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum number of temporary registers available in MIPS
#define MAX_TEMP_REGS 10

// MIPS temporary registers
const char* mipsTempRegs[MAX_TEMP_REGS] = {
    "$t0", "$t1", "$t2", "$t3", "$t4",
    "$t5", "$t6", "$t7", "$t8", "$t9"
};

// Structure to map temporaries to MIPS registers or memory
typedef struct TempMapEntry {
    char* tempName;       // e.g., "t1"
    char* mipsRegister;   // e.g., "$t0" or NULL if spilled to memory
    struct TempMapEntry* next;
} TempMapEntry;

// Head of the temporary mapping linked list
static TempMapEntry* tempMapHead = NULL;

// Output file pointer
static FILE* asmFile = NULL;

// Function to add a temporary mapping
void addTempMapping(const char* temp, const char* reg) {
    TempMapEntry* newEntry = (TempMapEntry*)malloc(sizeof(TempMapEntry));
    newEntry->tempName = strdup(temp);
    newEntry->mipsRegister = reg ? strdup(reg) : NULL;
    newEntry->next = tempMapHead;
    tempMapHead = newEntry;
}

// Function to get the MIPS register for a temporary
const char* getMIPSRegister(const char* temp) {
    TempMapEntry* current = tempMapHead;
    while (current != NULL) {
        if (strcmp(current->tempName, temp) == 0) {
            return current->mipsRegister;
        }
        current = current->next;
    }
    return NULL;
}

// Function to initialize the temporary mappings
void initializeTempMappings(const TACList* list) {
    // Collect all temporaries from the TAC list
    // For simplicity, assume temporaries are named t0, t1, ..., tN
    // and are identified by starting with 't'
    TACInstruction* instr = list->head;
    int tempCount = 0;

    while (instr != NULL) {
        if (instr->result && instr->result[0] == 't') {
            // Check if already mapped
            if (getMIPSRegister(instr->result) == NULL) {
                if (tempCount < MAX_TEMP_REGS) {
                    addTempMapping(instr->result, mipsTempRegs[tempCount]);
                } else {
                    // Spill to memory by allocating a label in .data
                    char label[50];
                    sprintf(label, "%s_mem", instr->result);
                    addTempMapping(instr->result, NULL); // NULL indicates spilled
                    // Declare in .data section
                    fprintf(asmFile, "%s: .word 0\n", label);
                }
                tempCount++;
            }
        }
        // Also check arg1 and arg2 for temporaries
        if (instr->arg1 && instr->arg1[0] == 't') {
            if (getMIPSRegister(instr->arg1) == NULL) {
                if (tempCount < MAX_TEMP_REGS) {
                    addTempMapping(instr->arg1, mipsTempRegs[tempCount]);
                } else {
                    char label[50];
                    sprintf(label, "%s_mem", instr->arg1);
                    addTempMapping(instr->arg1, NULL);
                    fprintf(asmFile, "%s: .word 0\n", label);
                }
                tempCount++;
            }
        }
        if (instr->arg2 && instr->arg2[0] == 't') {
            if (getMIPSRegister(instr->arg2) == NULL) {
                if (tempCount < MAX_TEMP_REGS) {
                    addTempMapping(instr->arg2, mipsTempRegs[tempCount]);
                } else {
                    char label[50];
                    sprintf(label, "%s_mem", instr->arg2);
                    addTempMapping(instr->arg2, NULL);
                    fprintf(asmFile, "%s: .word 0\n", label);
                }
                tempCount++;
            }
        }
        instr = instr->next;
    }
}

// Function to load a variable or temporary into a register
// Returns the register containing the value
const char* loadOperand(const char* operand) {
    // Check if operand is a constant
    char* endptr;
    int value = strtol(operand, &endptr, 10);
    if (*endptr == '\0') {
        // It's a constant, load into a temporary register (use $t9 as temporary)
        fprintf(asmFile, "    li $t9, %s\n", operand);
        return "$t9";
    }

    // It's a variable or temporary
    const char* reg = getMIPSRegister(operand);
    if (reg != NULL) {
        return reg;
    } else {
        // It's a spilled temporary, load from memory into $t9
        fprintf(asmFile, "    lw $t9, %s_mem\n", operand);
        return "$t9";
    }
}

// Function to store a register value back to memory if needed
void storeRegister(const char* temp, const char* reg) {
    const TempMapEntry* current = tempMapHead;
    while (current != NULL) {
        if (strcmp(current->tempName, temp) == 0) {
            if (current->mipsRegister == NULL) {
                // Spilled to memory, store $t9 into memory
                fprintf(asmFile, "    sw $t9, %s_mem\n", temp);
            }
            // If mapped to a register, no need to store
            return;
        }
        current = current->next;
    }
}

// Initialize the code generator by opening the output file and writing the header
int initializeCodeGenerator(const char* filename) {
    asmFile = fopen(filename, "w");
    if (asmFile == NULL) {
        perror("Failed to open output file");
        return 0;
    }

    // Write the MIPS header
    fprintf(asmFile, ".data\n");
    // Assuming variables are declared here; for simplicity, we declare a label for write syscall
    fprintf(asmFile, "newline: .asciiz \"\\n\"\n");
    // The spilled temporaries will be declared during temp mapping

    fprintf(asmFile, "\n.text\n");
    fprintf(asmFile, "    .globl main\n");
    fprintf(asmFile, "main:\n");

    return 1;
}

// Finalize the code generator by writing the exit syscall and closing the file
void finalizeCodeGenerator() {
    // Exit syscall
    fprintf(asmFile, "    li $v0, 10\n");
    fprintf(asmFile, "    syscall\n");

    if (asmFile != NULL) {
        fclose(asmFile);
        asmFile = NULL;
    }

    // Free temporary mappings
    TempMapEntry* current = tempMapHead;
    while (current != NULL) {
        TempMapEntry* temp = current;
        current = current->next;
        free(temp->tempName);
        if (temp->mipsRegister)
            free(temp->mipsRegister);
        free(temp);
    }
    tempMapHead = NULL;
}

// Generate MIPS code from the optimized TAC list
void generateMIPS(const TACList* list) {
    if (asmFile == NULL) {
        fprintf(stderr, "Error: Code generator not initialized.\n");
        return;
    }

    // Initialize temporary mappings and declare spilled temporaries
    initializeTempMappings(list);

    // Iterate through the TAC instructions and translate to MIPS
    TACInstruction* instr = list->head;
    while (instr != NULL) {
        switch (instr->op) {
            case TAC_ASSIGN: {
                // result = arg1
                // Load arg1 into a register
                const char* srcReg = loadOperand(instr->arg1);
                // Get destination register or handle memory
                const char* destReg = getMIPSRegister(instr->result);
                if (destReg != NULL) {
                    if (strcmp(srcReg, destReg) != 0) {
                        fprintf(asmFile, "    move %s, %s\n", destReg, srcReg);
                    }
                } else {
                    // Spilled to memory
                    fprintf(asmFile, "    sw %s, %s_mem\n", srcReg, instr->result);
                }
                break;
            }

            case TAC_ADD:
            case TAC_SUB:
            case TAC_MUL:
            case TAC_DIV: {
                // result = arg1 op arg2
                const char* reg1 = loadOperand(instr->arg1);
                const char* reg2 = loadOperand(instr->arg2);
                const char* destReg = getMIPSRegister(instr->result);

                if (destReg == NULL) {
                    // Use $t9 for computation and store later
                    destReg = "$t9";
                }

                switch (instr->op) {
                    case TAC_ADD:
                        fprintf(asmFile, "    add %s, %s, %s\n", destReg, reg1, reg2);
                        break;
                    case TAC_SUB:
                        fprintf(asmFile, "    sub %s, %s, %s\n", destReg, reg1, reg2);
                        break;
                    case TAC_MUL:
                        fprintf(asmFile, "    mul %s, %s, %s\n", destReg, reg1, reg2);
                        break;
                    case TAC_DIV:
                        fprintf(asmFile, "    div %s, %s\n", reg1, reg2);
                        fprintf(asmFile, "    mflo %s\n", destReg);
                        break;
                    default:
                        break;
                }

                if (getMIPSRegister(instr->result) == NULL) {
                    // Store the result from $t9 to memory
                    fprintf(asmFile, "    sw %s, %s_mem\n", destReg, instr->result);
                }

                break;
            }

            case TAC_WRITE: {
                // write arg1
                // Load arg1 into $a0
                const char* srcReg = loadOperand(instr->arg1);
                if (strcmp(srcReg, "$a0") != 0) {
                    fprintf(asmFile, "    move $a0, %s\n", srcReg);
                }
                // Prepare syscall for print integer
                fprintf(asmFile, "    li $v0, 1\n");
                fprintf(asmFile, "    syscall\n");
                // Print newline
                fprintf(asmFile, "    li $v0, 4\n");
                fprintf(asmFile, "    la $a0, newline\n");
                fprintf(asmFile, "    syscall\n");
                break;
            }

            default:
                fprintf(stderr, "Warning: Unsupported TAC operation.\n");
                break;
        }

        instr = instr->next;
    }
}

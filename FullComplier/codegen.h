// codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantic.h"

// Initialize the code generator (e.g., open output file)
int initializeCodeGenerator(const char* filename);

// Generate MIPS code from the optimized TAC list
void generateMIPS(const TACList* list);

// Finalize the code generator (e.g., close output file, free resources)
void finalizeCodeGenerator();

#endif // CODEGEN_H

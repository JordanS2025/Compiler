#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "semantic.h"

// Initialize the optimizer (if needed)
void initializeOptimizer();

// Perform constant folding, constant propagation, and dead code elimination on the TAC list
void optimizeTAC();

// Print the optimized TAC
void printOptimizedTAC(const TACList* list);

// Finalize the optimizer (free resources if needed)
void finalizeOptimizer();

#endif // OPTIMIZER_H

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/*
 * Generate Three-Address Code (TAC) for the given AST.
 */
void generate_tac(ASTNode* root);

/* A simple temporary register allocator */
char* new_temp();

/* A simple label allocator */
char* new_label();

#endif /* CODEGEN_H */

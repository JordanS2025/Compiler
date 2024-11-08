/* codegen.h */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symbol_table.h"
// In codegen.h or a relevant header file

// Initialize the TAC generator
void init_TAC();

// Finalize the TAC generator
void finalize_TAC();

// Generate TAC by traversing the AST
void generate_TAC(ASTNode *root, SymbolTable *sym_table);

// Traverse AST and generate TAC, returns the temporary holding the result
char* traverse_AST(ASTNode *node, SymbolTable *sym_table);

// Function to map a variable to a temporary
char* map_var_to_temp(const char *var_name, SymbolTable *sym_table);

// Function to set a variable to a temporary (used for arrays)
void set_var_to_temp(const char *var_name, const char *temp_name);

#endif /* CODEGEN_H */

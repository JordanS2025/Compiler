/* semantic.h */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "symbol_table.h"
#include "ast.h"

// Function prototypes
bool perform_semantic_analysis(ASTNode* root, SymbolTable* sym_table);
const char* get_expression_type(ASTNode* node, SymbolTable* sym_table);
void check_array_type_consistency(ASTNode* array_init, const char* expected_type, SymbolTable* sym_table);

#endif // SEMANTIC_H

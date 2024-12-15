#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

/* 
 * Traverse the AST and perform semantic checks.
 * This will report errors for:
 * - Type mismatches in expressions (e.g., int + float)
 * - Invalid array initializations (e.g., too many elements for declared size)
 * - Invalid condition comparisons in if/while (operands must match in type)
 */
void traverse_ast(ASTNode* root);

/* 
 * Check expressions for type correctness.
 * This will annotate AST nodes with their resulting type.
 * Will print errors if type mismatches occur.
 */
DataType check_expression(ASTNode* expr);

/* 
 * Check array initialization to ensure the number of elements matches the array size.
 * Will print errors if mismatch occurs.
 */
void check_array_initialization(ASTNode* declaration_node);

/* 
 * Utility function to print semantic errors.
 */
void report_semantic_error(const char* format, ...);

#endif /* SEMANTIC_H */

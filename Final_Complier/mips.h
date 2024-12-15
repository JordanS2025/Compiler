#ifndef MIPS_H
#define MIPS_H

#include "ast.h"

void generate_mips(ASTNode* root);
void generate_write(ASTNode* node);
void generate_function_call(ASTNode* node);
void generate_function_parameters(ASTNode* params);
void generate_data_section(ASTNode* root);
void generate_text_section(ASTNode* root);

#endif

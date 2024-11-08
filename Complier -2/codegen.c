#include "codegen.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global temporary counter
static int temp_count = 0;
extern int line_num;

// Current array name being initialized
static char *current_array_name = NULL;

// Current index for array initialization
static int current_index = 0;

// Get next available temporary register (t0-t9)
static int get_next_temp_reg() {
    return (temp_count++ % 10);
}

// Get temporary register name
static char* get_temp_reg() {
    static char reg_name[5];
    sprintf(reg_name, "$t%d", get_next_temp_reg());
    return reg_name;
}

// Symbol table mapping variables to temporaries
typedef struct VarTempMap {
    char *var_name;
    char *temp_name;
    struct VarTempMap *next;
} VarTempMap;

// Head of the VarTempMap linked list
static VarTempMap *var_temp_head = NULL;

// Define TACInstruction structure
typedef struct TACInstruction {
    char *instruction;
    struct TACInstruction *next;
} TACInstruction;

// Global pointers for TAC list
static TACInstruction *tac_head = NULL;
static TACInstruction *tac_tail = NULL;

// Mapping from array names to base registers
typedef struct ArrayRegMap {
    char *array_name;
    char *reg_name;
    struct ArrayRegMap *next;
} ArrayRegMap;

static ArrayRegMap *array_reg_head = NULL;
static int array_reg_count = 0;

// Helper function to check if a variable name is a temporary
int is_temporary(const char *var_name) {
    return (var_name[0] == 't' && isdigit(var_name[1]));
}

// Initialize TAC generator
void init_TAC() {
    tac_head = NULL;
    tac_tail = NULL;
    temp_count = 0;
    array_reg_count = 0;
}

// Function to add TAC instruction to the list
void add_TAC_instruction(const char *instr) {
    TACInstruction *new_instr = (TACInstruction*)malloc(sizeof(TACInstruction));
    if (!new_instr) {
        fprintf(stderr, "Error: Memory allocation failed for TACInstruction.\n");
        exit(1);
    }
    new_instr->instruction = strdup(instr);
    new_instr->next = NULL;

    if (tac_head == NULL) {
        tac_head = new_instr;
        tac_tail = new_instr;
    } else {
        tac_tail->next = new_instr;
        tac_tail = new_instr;
    }
}

// Generate a new temporary and return its name with '$' prefix
char* new_temp() {
    char buffer[10];
    sprintf(buffer, "$t%d", get_next_temp_reg());
    return strdup(buffer);
}

// Function to translate TAC to assembly
void translate_TAC_to_assembly(SymbolTable *sym_table) {
    FILE *asm_fp = fopen("output.asm", "w");
    if (!asm_fp) {
        fprintf(stderr, "Error: Could not open output.asm for writing.\n");
        exit(1);
    }

    // Write data section
    fprintf(asm_fp, ".data\n");
    Symbol *current_sym = sym_table->head;
    while (current_sym != NULL) {
        if (!current_sym->is_function) {
            if (current_sym->is_array) {
                fprintf(asm_fp, "%s: .word ", current_sym->name);
                int total_size = 1;
                for (int i = 0; i < current_sym->num_dimensions; i++) {
                    total_size *= current_sym->sizes[i];
                }
                for (int i = 0; i < total_size; i++) {
                    fprintf(asm_fp, "0%s", i < total_size - 1 ? ", " : "");
                }
                fprintf(asm_fp, "\n");
            } else {
                fprintf(asm_fp, "%s: .word 0\n", current_sym->name);
            }
        }
        current_sym = current_sym->next;
    }

    // Write text section
    fprintf(asm_fp, "\n.text\n");
    fprintf(asm_fp, ".globl main\n\n");

    // Process TAC instructions
    TACInstruction *current = tac_head;
    while (current != NULL) {
        char *instr = current->instruction;
        char lhs[100], rhs[100];

        if (strcmp(instr, "func_main:") == 0) {
            fprintf(asm_fp, "main:\n");
        } else if (strncmp(instr, "func_", 5) == 0) {
            fprintf(asm_fp, "%s\n", instr);
        } else if (sscanf(instr, "%s = %[^\n]", lhs, rhs) == 2) {
            // Handle multiplication
            if (strstr(rhs, "*") != NULL) {
                char op1[100], op2[100];
                sscanf(rhs, "%s * %s", op1, op2);
                fprintf(asm_fp, "    mul %s, %s, %s\n", lhs, op1, op2);
            }
            // Handle addition
            else if (strstr(rhs, "+") != NULL) {
                char op1[100], op2[100];
                sscanf(rhs, "%s + %s", op1, op2);
                fprintf(asm_fp, "    add %s, %s, %s\n", lhs, op1, op2);
            }
            // Handle array assignments
            else if (strstr(lhs, "[") != NULL) {
                char array_name[50], index[50];
                sscanf(lhs, "%[^[][%[^]]", array_name, index);
                
                fprintf(asm_fp, "    la $t9, %s\n", array_name);
                if (!strncmp(rhs, "$", 1)) {
                    fprintf(asm_fp, "    move $t8, %s\n", rhs);
                } else {
                    fprintf(asm_fp, "    li $t8, %s\n", rhs);
                }
                
                if (isdigit(index[0])) {
                    fprintf(asm_fp, "    sw $t8, %d($t9)\n", 4 * atoi(index));
                } else {
                    fprintf(asm_fp, "    sll $t7, %s, 2\n", index);
                    fprintf(asm_fp, "    add $t9, $t9, $t7\n");
                    fprintf(asm_fp, "    sw $t8, 0($t9)\n");
                }
            }
            // Handle array access
            else if (strstr(rhs, "[") != NULL) {
                char array_name[50], index[50];
                sscanf(rhs, "%[^[][%[^]]", array_name, index);
                
                fprintf(asm_fp, "    la $t9, %s\n", array_name);
                if (isdigit(index[0])) {
                    fprintf(asm_fp, "    lw %s, %d($t9)\n", lhs, 4 * atoi(index));
                } else {
                    fprintf(asm_fp, "    sll $t7, %s, 2\n", index);
                    fprintf(asm_fp, "    add $t9, $t9, $t7\n");
                    fprintf(asm_fp, "    lw %s, 0($t9)\n", lhs);
                }
            }
            // Handle immediate values
            else if (isdigit(rhs[0]) || (rhs[0] == '-' && isdigit(rhs[1]))) {
                fprintf(asm_fp, "    li %s, %s\n", lhs, rhs);
            }
            // Handle register moves
            else {
                if (!strncmp(rhs, "$", 1)) {
                    fprintf(asm_fp, "    move %s, %s\n", lhs, rhs);
                } else {
                    fprintf(asm_fp, "    lw $t8, %s\n", rhs);
                    fprintf(asm_fp, "    move %s, $t8\n", lhs);
                }
            }
        }
        // Handle write statements
        else if (strncmp(instr, "write ", 6) == 0) {
            char var[100];
            sscanf(instr, "write %s", var);
            if (!strncmp(var, "$", 1)) {
                fprintf(asm_fp, "    move $a0, %s\n", var);
            } else {
                fprintf(asm_fp, "    lw $a0, %s\n", var);
            }
            fprintf(asm_fp, "    li $v0, 1\n");
            fprintf(asm_fp, "    syscall\n");
            fprintf(asm_fp, "    li $v0, 11\n");
            fprintf(asm_fp, "    li $a0, 10\n");
            fprintf(asm_fp, "    syscall\n");
        }
        current = current->next;
    }

    fclose(asm_fp);
}


void set_var_to_temp(const char *var_name, const char *temp_name) {
    if (var_name == NULL || temp_name == NULL) {
        fprintf(stderr, "Error: Variable name or temp name is NULL in set_var_to_temp.\n");
        exit(1);
    }

    VarTempMap *current = var_temp_head;
    while (current != NULL) {
        if (strcmp(current->var_name, var_name) == 0) {
            free(current->temp_name);
            current->temp_name = strdup(temp_name);
            return;
        }
        current = current->next;
    }

    VarTempMap *new_map = (VarTempMap*)malloc(sizeof(VarTempMap));
    new_map->var_name = strdup(var_name);
    new_map->temp_name = strdup(temp_name);
    new_map->next = var_temp_head;
    var_temp_head = new_map;
}

ASTNode* find_ArrayInit(ASTNode *node) {
    while (node != NULL) {
        if (strcmp(node->node_type, "ArrayInit") == 0) {
            return node;
        }
        if (strcmp(node->node_type, "Initializer") == 0) {
            node = node->left;
        } else {
            break;
        }
    }
    return NULL;
}

char* map_var_to_temp(const char *var_name, SymbolTable *sym_table) {
    if (is_temporary(var_name)) {
        char buffer[10];
        sprintf(buffer, "$%s", var_name);
        return strdup(buffer);
    }

    Symbol *sym = get_symbol(sym_table, var_name);
    if (sym && sym->is_array) {
        return strdup(var_name);
    }

    VarTempMap *current = var_temp_head;
    while (current != NULL) {
        if (strcmp(current->var_name, var_name) == 0)
            return strdup(current->temp_name);
        current = current->next;
    }

    char *temp = new_temp();
    VarTempMap *new_map = (VarTempMap*)malloc(sizeof(VarTempMap));
    new_map->var_name = strdup(var_name);
    new_map->temp_name = strdup(temp);
    new_map->next = var_temp_head;
    var_temp_head = new_map;

    return temp;
}

void finalize_TAC() {
    TACInstruction *current = tac_head;
    while (current != NULL) {
        TACInstruction *temp = current;
        current = current->next;
        free(temp->instruction);
        free(temp);
    }
    tac_head = NULL;
    tac_tail = NULL;

    VarTempMap *current_var = var_temp_head;
    while (current_var != NULL) {
        VarTempMap *temp_var = current_var;
        current_var = current_var->next;
        free(temp_var->var_name);
        free(temp_var->temp_name);
        free(temp_var);
    }
    var_temp_head = NULL;
}

    // Function to traverse AST and generate TAC
    char* traverse_AST(ASTNode *node, SymbolTable *sym_table) {
        if (node == NULL) {
            return NULL; // Skip processing for NULL nodes
        }

        // Debugging: Print the current node being processed
        //printf("Processing node_type: %s, value: %s\n", node->node_type, node->value ? node->value : "NULL");

        // Handle different node types
        if (strcmp(node->node_type, "Program") == 0 ||
            strcmp(node->node_type, "Declarations") == 0 ||
            strcmp(node->node_type, "Statements") == 0 ||
            strcmp(node->node_type, "VarDeclList") == 0 ||
            strcmp(node->node_type, "StmtList") == 0 ||
            strcmp(node->node_type, "FunctionDefList") == 0 ||
            strcmp(node->node_type, "Type") == 0) { // Handle Type nodes by traversing children
            traverse_AST(node->left, sym_table);
            traverse_AST(node->right, sym_table);
            return NULL; // No specific temporary for these nodes
        }
        else if (strcmp(node->node_type, "ArraySizes") == 0) {
            // No action needed for array sizes in TAC
            traverse_AST(node->left, sym_table);
            traverse_AST(node->right, sym_table);
            return NULL;
        }
        else if (strcmp(node->node_type, "FunctionDef") == 0) {
            // Emit function label with "func_" prefix
            // e.g., "func_sum:"
            char buffer[100];
            sprintf(buffer, "func_%s:", node->value); // Added "func_" prefix
            add_TAC_instruction(buffer);

            // Traverse function body
            traverse_AST(node->right, sym_table);

            return NULL;
        }
        else if (strcmp(node->node_type, "MainFunction") == 0) {
            // Emit main function label
            // e.g., func_main:
            add_TAC_instruction("func_main:");

            // Traverse main function body
            traverse_AST(node->left, sym_table);

            return NULL;
        }
        else if (strcmp(node->node_type, "FunctionBody") == 0 ||
                 strcmp(node->node_type, "MainBody") == 0) {
            traverse_AST(node->left, sym_table);  // Declarations
            traverse_AST(node->right, sym_table); // Statements and Return
            return NULL;
        }
        else if (strcmp(node->node_type, "VarDeclList") == 0) {
            // Traverse each VarDecl in the list
            traverse_AST(node->left, sym_table);  // VarDecl or ArrayDecl
            traverse_AST(node->right, sym_table); // Next VarDeclList
            return NULL;
        }
        else if (strcmp(node->node_type, "VarDecl") == 0) {
            // Handle scalar variable declarations
            Symbol *sym = get_symbol(sym_table, node->value);
            if (sym == NULL) {
                fprintf(stderr, "Error: Variable '%s' not found in symbol table at line %d.\n", node->value, line_num);
                exit(1);
            }

            if (sym->is_array) {
                // This VarDecl is actually an ArrayDecl, but handled separately
                // To avoid redundancy, skip processing here
                return NULL;
            } else {
                // Handle scalar variable
                char *expr_temp = traverse_AST(node->right, sym_table); // Initializer node
                set_var_to_temp(node->value, expr_temp); // Direct mapping

                if (expr_temp == NULL) {
                    fprintf(stderr, "Error: Initialization expression for variable '%s' evaluated to NULL, in VarDecl at line %d.\n", node->value, line_num);
                    exit(1);
                }
            }
            return NULL;
        }
        else if (strcmp(node->node_type, "ArrayDecl") == 0) {
            // Retrieve array name
            char *array_name = node->value;

            // Navigate to ArraySizes node
            if (node->right == NULL || strcmp(node->right->node_type, "ArraySizes") != 0) {
                fprintf(stderr, "Error: Array '%s' does not have ArraySizes node at line %d.\n", array_name, line_num);
                exit(1);
            }

            ASTNode *array_sizes_node = node->right;

            // Extract array size from Number node
            if (array_sizes_node->left == NULL || strcmp(array_sizes_node->left->node_type, "Number") != 0) {
                fprintf(stderr, "Error: Array '%s' does not have a valid size at line %d.\n", array_name, line_num);
                exit(1);
            }

            int array_size = atoi(array_sizes_node->left->value);

            // Navigate to Initializer node
            if (array_sizes_node->right == NULL || strcmp(array_sizes_node->right->node_type, "Initializer") != 0) {
                fprintf(stderr, "Error: Array '%s' must be initialized with an initializer list at line %d.\n", array_name, line_num);
                exit(1);
            }

            ASTNode *initializer_node = array_sizes_node->right;

            // Use the helper function to find the ArrayInit node
            ASTNode *array_init_node = find_ArrayInit(initializer_node->left);
            if (array_init_node == NULL) {
                fprintf(stderr, "Error: Array '%s' must be initialized with an ArrayInit node at line %d.\n", array_name, line_num);
                exit(1);
            }

            // Set up for array initialization
            current_array_name = strdup(array_name);
            if (current_array_name == NULL) {
                fprintf(stderr, "Error: Memory allocation failed for current_array_name.\n");
                exit(1);
            }

            current_index = 0;

            // Traverse the ArrayInit node to assign values to each array element
            traverse_AST(array_init_node, sym_table); // Process array elements

            // Reset flags and free current_array_name
            free(current_array_name);
            current_array_name = NULL;

            // Map the array variable to its name (no temporary)
            set_var_to_temp(array_name, array_name); // Direct mapping to array name

            return NULL;
        }
        else if (strcmp(node->node_type, "Initializer") == 0) {
            if (node->right == NULL) {
                // Single initializer (e.g., b = 3)
                char* temp = traverse_AST(node->left, sym_table);
                return temp;
            } else {
                // Multiple initializers (handled via Initializer list)
                traverse_AST(node->left, sym_table);
                traverse_AST(node->right, sym_table);
                return NULL;
            }
        }
        else if (strcmp(node->node_type, "ArrayInit") == 0) {
            // Handle ArrayInit node by assigning each initializer to array elements
            ASTNode *current = node->left;
            while (current != NULL) {
                if (strcmp(current->node_type, "Initializer") != 0) {
                    fprintf(stderr, "Error: Expected Initializer node in ArrayInit for array '%s' at line %d.\n", current_array_name, line_num);
                    exit(1);
                }

                if (current->left == NULL) {
                    fprintf(stderr, "Error: Initializer node in ArrayInit for array '%s' lacks value at line %d.\n", current_array_name, line_num);
                    exit(1);
                }

                char *value = current->left->value;
                if (value == NULL) {
                    fprintf(stderr, "Error: Initializer value is NULL in ArrayInit for array '%s' at line %d.\n", current_array_name, line_num);
                    exit(1);
                }

                // Emit array element assignment
                char buffer[256];
                sprintf(buffer, "%s[%d] = %s", current_array_name, current_index, value);
                add_TAC_instruction(buffer);
                current_index++;

                // Move to the next Initializer
                current = current->right;
            }

            return NULL;
        }
        else if (strcmp(node->node_type, "Assignment") == 0) {
            // Handle scalar assignment
            char *expr_temp = traverse_AST(node->right, sym_table); // expression
            char *x_temp = map_var_to_temp(node->value, sym_table); // map x to temp
            if (expr_temp == NULL) {
                fprintf(stderr, "Error: Assignment expression for variable '%s' evaluated to NULL at line %d.\n", node->value, line_num);
                exit(1);
            }
            char buffer[256];
            sprintf(buffer, "%s = %s", x_temp, expr_temp);
            add_TAC_instruction(buffer);
            return NULL;
        }
        else if (strcmp(node->node_type, "ArrayAssignment") == 0) {
            // Handle array element assignment
            char *array_name = node->value;
            char *index_temp = traverse_AST(node->left, sym_table); // index expression
            char *expr_temp = traverse_AST(node->right, sym_table); // value expression
            if (index_temp == NULL || expr_temp == NULL) {
                fprintf(stderr, "Error: Array assignment indices or expression evaluated to NULL at line %d.\n", line_num);
                exit(1);
            }
            char buffer[256];
            sprintf(buffer, "%s[%s] = %s", array_name, index_temp, expr_temp);
            add_TAC_instruction(buffer);
            return NULL;
        }
        else if (strcmp(node->node_type, "Write") == 0) {
            // Handle write statements
            printf("Processing Write node at line %d\n", line_num); // Debugging

            char *expr_temp = NULL;

            if (node->left != NULL) {
                printf("Write node's left child: type=%s, value=%s\n", 
                       node->left->node_type, 
                       node->left->value ? node->left->value : "NULL");
                expr_temp = traverse_AST(node->left, sym_table); // expression to write
            } else if (node->value != NULL) {
                printf("Write node has no left child. Using node->value: %s\n", 
                       node->value ? node->value : "NULL");
                
                // Create a temporary AST node representing the expression
                ASTNode temp_node;
                temp_node.node_type = "ID"; // Assuming the expression is an identifier
                temp_node.value = node->value;
                temp_node.left = NULL;
                temp_node.right = NULL;
                
                expr_temp = traverse_AST(&temp_node, sym_table);
            } else {
                fprintf(stderr, "Error: Write expression evaluated to NULL at line %d.\n", line_num);
                exit(1);
            }

            if (expr_temp == NULL) {
                fprintf(stderr, "Error: Write expression evaluated to NULL at line %d.\n", line_num);
                exit(1);
            }
            
            // Replace fprintf with add_TAC_instruction
            char buffer[256];
            sprintf(buffer, "write %s", expr_temp);
            add_TAC_instruction(buffer);
            
            printf("traverse_AST: Generated Write statement for temp '%s'\n", expr_temp);

            // Traverse the right child to handle subsequent statements
            traverse_AST(node->right, sym_table);
            return NULL;
        }
        else if (strcmp(node->node_type, "BinaryOp") == 0) {
            // Generate TAC for binary operations
            char *left_temp = traverse_AST(node->left, sym_table);
            char *right_temp = traverse_AST(node->right, sym_table);
            if (left_temp == NULL || right_temp == NULL) {
                fprintf(stderr, "Error: Binary operation operands evaluated to NULL at line %d.\n", line_num);
                exit(1);
            }
            char *result_temp = new_temp();
            char buffer[256];
            sprintf(buffer, "%s = %s %s %s", result_temp, left_temp, node->value, right_temp);
            add_TAC_instruction(buffer);
            return result_temp;
        }
        else if (strcmp(node->node_type, "Number") == 0 || strcmp(node->node_type, "Float") == 0) {
            if (current_array_name != NULL) {
                // In array initialization, handle directly in ArrayInit node
                // Thus, no action needed here
                return NULL;
            }

            // Handle numeric literals as expressions
            if (node->value == NULL) {
                fprintf(stderr, "Error: Numeric literal has NULL value at line %d.\n", line_num);
                exit(1);
            }
            char *temp = new_temp();
            char buffer[256];
            sprintf(buffer, "%s = %s", temp, node->value);
            add_TAC_instruction(buffer);
            return temp;
        }
        else if (strcmp(node->node_type, "ID") == 0) {
            // Replace variable with its corresponding temporary
            printf("Processing ID node: %s\n", node->value); // Debugging
            char *temp = map_var_to_temp(node->value, sym_table);
            if (temp == NULL) {
                fprintf(stderr, "Error: Variable '%s' not found in symbol table at line %d.\n", node->value, line_num);
                exit(1);
            }
            printf("map_var_to_temp: '%s' mapped to '%s'\n", node->value, temp); // Debugging
            return temp;
        }
        else if (strcmp(node->node_type, "ArrayAccess") == 0) {
            // Handle array access
            char *array_name = node->value; // the array name

            // Traverse the index expression
            char *index_temp = traverse_AST(node->left, sym_table); // index expression

            if (index_temp == NULL) {
                fprintf(stderr, "Error: Array index expression evaluated to NULL at line %d.\n", line_num);
                exit(1);
            }

            // Generate TAC to access the array element
            char *result_temp = new_temp();
            char buffer[256];
            sprintf(buffer, "%s = %s[%s]", result_temp, array_name, index_temp);
            add_TAC_instruction(buffer);
            return result_temp;
        }
        else if (strcmp(node->node_type, "FunctionCall") == 0) {
            // Handle function calls
            // Assuming no parameters for simplicity
            char *return_temp = new_temp();
            char buffer[256];
            sprintf(buffer, "call %s", node->value);
            add_TAC_instruction(buffer);

            // **Important:** Traverse the right child to handle subsequent statements
            traverse_AST(node->right, sym_table);

            return return_temp;
        }
        else if (strcmp(node->node_type, "Return") == 0) {
            // Handle return statements
            char *return_temp = traverse_AST(node->left, sym_table);
            if (return_temp == NULL) {
                fprintf(stderr, "Error: Return expression evaluated to NULL at line %d.\n", line_num);
                exit(1);
            }
            char buffer[256];
            sprintf(buffer, "return %s", return_temp);
            add_TAC_instruction(buffer);

            // **Note:** Typically, a return statement ends the function, so no need to traverse further
            return NULL;
        }
        else {
            fprintf(stderr, "Error: Unknown AST node type '%s' during TAC generation at line %d.\n", node->node_type, line_num);
            exit(1);
        }
    }

    // Function to generate TAC and translate to assembly
    void generate_TAC(ASTNode *root, SymbolTable *sym_table) {
        init_TAC();
        traverse_AST(root, sym_table);

        // After generating TAC, translate it to MIPS assembly
        translate_TAC_to_assembly(sym_table);

        // Now finalize TAC to free memory
        finalize_TAC();
    }
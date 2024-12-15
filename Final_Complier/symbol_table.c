#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global pointer to the top of the symbol table stack
static SymbolTable* current_table = NULL;

// Helper function to convert DataType enum to string
static const char* datatype_to_string(DataType type) {
    switch(type) {
        case DT_INT: return "int";
        case DT_FLOAT: return "float";
        case DT_CHAR: return "char";
        case DT_VOID: return "void";
        case DT_ARRAY: return "array";
        default: return "unknown";
    }
}

// Initialize the symbol table by creating the global scope
void init_symbol_table() {
    current_table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!current_table) {
        fprintf(stderr, "Failed to initialize symbol table.\n");
        exit(EXIT_FAILURE);
    }
    current_table->symbols = NULL;
    current_table->next = NULL;
    current_table->scope_level = 0;
    printf("Initialized global scope (Level 0).\n");
}

// Enter a new scope by pushing a new symbol table onto the stack
void enter_scope() {
    SymbolTable* new_table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!new_table) {
        fprintf(stderr, "Failed to enter new scope.\n");
        exit(EXIT_FAILURE);
    }
    new_table->symbols = NULL;
    new_table->next = current_table;
    new_table->scope_level = current_table->scope_level + 1;
    current_table = new_table;
    printf("Entered new scope level %d.\n", current_table->scope_level);
}

// Exit the current scope by popping the top symbol table from the stack
void exit_scope() {
    if (!current_table) {
        fprintf(stderr, "No scope to exit.\n");
        return;
    }
    SymbolTable* temp = current_table;
    current_table = current_table->next;

    // Free all symbols in the exiting scope
    Symbol* symbol = temp->symbols;
    while (symbol) {
        Symbol* to_free = symbol;
        symbol = symbol->next;
        // Free allocated memory for name
        free(to_free->name);
        // Free array sizes if it's an array
        if (to_free->category == SYMBOL_ARRAY && to_free->array_sizes) {
            free(to_free->array_sizes);
        }
        // Free parameters if it's a function
        if (to_free->category == SYMBOL_FUNCTION && to_free->params) {
            // Assuming parameters are stored as a linked list
            Symbol* param = to_free->params;
            while (param) {
                Symbol* param_free = param;
                param = param->next;
                free(param_free->name);
                free(param_free);
            }
        }
        free(to_free);
    }
    free(temp);
    printf("Exited to scope level %d.\n", current_table ? current_table->scope_level : -1);
}

// Add a new symbol to the current scope
bool add_symbol(char* name, DataType type, SymbolCategory category, 
               DataType return_type, Symbol* params, int* array_sizes, int dimensions) {
    if (!current_table) {
        fprintf(stderr, "Symbol table not initialized.\n");
        return false;
    }

    // Check if symbol already exists in the current scope
    Symbol* temp = current_table->symbols;
    while (temp) {
        if (strcmp(temp->name, name) == 0) {
            fprintf(stderr, "Symbol '%s' already declared in the current scope.\n", name);
            return false;
        }
        temp = temp->next;
    }

    // Create a new symbol
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    if (!new_symbol) {
        fprintf(stderr, "Failed to allocate memory for symbol '%s'.\n", name);
        return false;
    }
    new_symbol->name = strdup(name);
    new_symbol->type = type;
    new_symbol->category = category;
    new_symbol->scope_level = current_table->scope_level;
    new_symbol->params = params;
    new_symbol->return_type = return_type;
    new_symbol->array_sizes = NULL;
    new_symbol->dimensions = 0;

    if (category == SYMBOL_ARRAY) {
        new_symbol->array_sizes = (int*)malloc(sizeof(int) * dimensions);
        if (!new_symbol->array_sizes) {
            fprintf(stderr, "Failed to allocate memory for array sizes of '%s'.\n", name);
            free(new_symbol->name);
            free(new_symbol);
            return false;
        }
        memcpy(new_symbol->array_sizes, array_sizes, sizeof(int) * dimensions);
        new_symbol->dimensions = dimensions;
    }

    // Insert the new symbol at the beginning of the symbols list
    new_symbol->next = current_table->symbols;
    current_table->symbols = new_symbol;

    printf("Added symbol '%s' of type '%s' to scope level %d.\n", 
           name, datatype_to_string(type), current_table->scope_level);
    return true;
}

// Lookup a symbol by name, searching from the current scope upwards
Symbol* lookup_symbol(char* name) {
    SymbolTable* table = current_table;
    while (table) {
        Symbol* symbol = table->symbols;
        while (symbol) {
            if (strcmp(symbol->name, name) == 0) {
                return symbol;
            }
            symbol = symbol->next;
        }
        table = table->next;
    }
    return NULL; // Symbol not found
}

// Print the symbol table for debugging
void print_symbol_table() {
    SymbolTable* table = current_table;
    printf("======= Symbol Table =======\n");
    while (table) {
        printf("Scope Level %d:\n", table->scope_level);
        Symbol* symbol = table->symbols;
        while (symbol) {
            printf("  Name: %s, Type: ", symbol->name);
            switch(symbol->type) {
                case DT_INT: printf("int"); break;
                case DT_FLOAT: printf("float"); break;
                case DT_CHAR: printf("char"); break;
                case DT_VOID: printf("void"); break;
                case DT_ARRAY: printf("array"); break;
                default: printf("unknown"); break;
            }
            printf(", Category: ");
            switch(symbol->category) {
                case SYMBOL_VARIABLE: printf("Variable"); break;
                case SYMBOL_FUNCTION: printf("Function"); break;
                case SYMBOL_ARRAY: printf("Array"); break;
                default: printf("Unknown"); break;
            }
            if (symbol->category == SYMBOL_FUNCTION) {
                printf(", Return Type: ");
                switch(symbol->return_type) {
                    case DT_INT: printf("int"); break;
                    case DT_FLOAT: printf("float"); break;
                    case DT_CHAR: printf("char"); break;
                    case DT_VOID: printf("void"); break;
                    default: printf("unknown"); break;
                }
                // Print parameters
                if (symbol->params) {
                    printf(", Parameters: ");
                    Symbol* param = symbol->params;
                    while (param) {
                        printf("%s %s", datatype_to_string(param->type), param->name);
                        if (param->next) printf(", ");
                        param = param->next;
                    }
                }
            }
            if (symbol->category == SYMBOL_ARRAY) {
                printf(", Dimensions: %d, Sizes: ", symbol->dimensions);
                for(int i = 0; i < symbol->dimensions; i++) {
                    printf("%d", symbol->array_sizes[i]);
                    if (i < symbol->dimensions -1) printf("x");
                }
            }
            printf("\n");
            symbol = symbol->next;
        }
        table = table->next;
    }
    printf("============================\n");
}

// Free all symbol tables and their symbols
void free_all_symbol_tables() {
    while (current_table) {
        exit_scope();
    }
}

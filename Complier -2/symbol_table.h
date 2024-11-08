/* symbol_table.h */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

#define MAX_ARRAY_DIMENSIONS 10
#define MAX_FUNCTION_PARAMS 10

typedef struct Parameter {
    char* type;
    char* name;
} Parameter;

typedef struct Symbol {
    char* name;
    char* type;
    bool is_array;
    bool is_function;
    bool initialized;
    int num_dimensions;
    int sizes[MAX_ARRAY_DIMENSIONS];
    int num_parameters;
    Parameter parameters[MAX_FUNCTION_PARAMS];
    struct Symbol* next;
} Symbol;

typedef struct SymbolTable {
    Symbol* head;
} SymbolTable;

// Function prototypes
SymbolTable* create_symbol_table();
bool add_symbol(SymbolTable* table, const char* name, const char* type);
bool add_array_symbol(SymbolTable* table, const char* name, const char* type, int num_dimensions, int sizes[]);
bool add_function_symbol(SymbolTable* table, const char* name, const char* return_type);
bool add_function_symbol_with_params(SymbolTable* table, const char* name, const char* return_type, int num_params, Parameter params[]);
bool symbol_exists(SymbolTable* table, const char* name);
bool function_exists(SymbolTable* table, const char* name);
Symbol* get_symbol(SymbolTable* table, const char* name);
void free_symbol_table(SymbolTable* table);
void print_symbol_table(SymbolTable* table);

#endif // SYMBOL_TABLE_H

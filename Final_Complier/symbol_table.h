#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// Enumeration for basic data types with a distinct prefix to avoid conflicts
typedef enum {
    DT_INT,
    DT_FLOAT,
    DT_CHAR,
    DT_VOID,
    DT_ARRAY
} DataType;

// Enumeration for symbol categories
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_ARRAY
} SymbolCategory;

// Forward declaration for Symbol to handle parameters
typedef struct Symbol Symbol;

// Structure to hold information about a symbol
struct Symbol {
    char* name;                     // Symbol name
    DataType type;                  // Data type
    SymbolCategory category;        // Symbol category
    int scope_level;                // Scope level where the symbol is defined

    // For functions
    Symbol* params;                 // Linked list of parameters (if function)
    DataType return_type;           // Return type (if function)

    // For arrays
    int* array_sizes;               // Array sizes (if array)
    int dimensions;                 // Number of dimensions

    Symbol* next;                   // Pointer to the next symbol in the list
};

// Structure to represent array sizes
typedef struct ArraySizeNode {
    int* sizes;
    int dimensions;
} ArraySizeNode;

// Structure for the symbol table (a stack of scopes)
typedef struct SymbolTable {
    Symbol* symbols;                // Linked list of symbols in the current scope
    struct SymbolTable* next;       // Pointer to the next scope in the stack
    int scope_level;                // Current scope level
} SymbolTable;

// Initialize the symbol table
void init_symbol_table();

// Enter a new scope
void enter_scope();

// Exit the current scope
void exit_scope();

// Add a new symbol to the symbol table
bool add_symbol(char* name, DataType type, SymbolCategory category, 
               DataType return_type, Symbol* params, int* array_sizes, int dimensions);

// Lookup a symbol in the symbol table
Symbol* lookup_symbol(char* name);

// Print the symbol table for debugging
void print_symbol_table();

// Free all memory allocated for the symbol table
void free_all_symbol_tables();

#endif // SYMBOL_TABLE_H

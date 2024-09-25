#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>

// Define the structure for a symbol table entry
typedef struct SymbolTableEntry {
    char* name;             // Variable name
    char* type;             // Variable type ("int" or "char")
    int line;               // Declaration line number
    struct SymbolTableEntry* next; // Pointer to the next entry
} SymbolTableEntry;

// Define the symbol table as a pointer to the first entry
typedef struct {
    SymbolTableEntry* head;
} SymbolTable;

// Function prototypes
void initializeSymbolTable(SymbolTable* table);
int addSymbol(SymbolTable* table, const char* name, const char* type, int line);
SymbolTableEntry* findSymbol(SymbolTable* table, const char* name);
void printSymbolTable(const SymbolTable* table);
void freeSymbolTable(SymbolTable* table);

#endif // SYMBOLTABLE_H


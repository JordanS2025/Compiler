#include "symboltable.h"
#include <stdlib.h>
#include <string.h>

// Initialize the symbol table
void initializeSymbolTable(SymbolTable* table) {
    table->head = NULL;
}

// Add a symbol to the symbol table
// Returns 0 on success, -1 if the symbol already exists
int addSymbol(SymbolTable* table, const char* name, const char* type, int line) {
    // Check if the symbol already exists
    if (findSymbol(table, name) != NULL) {
        return -1; // Symbol already declared
    }

    // Create a new symbol table entry
    SymbolTableEntry* newEntry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    if (!newEntry) {
        fprintf(stderr, "Memory allocation failed for symbol '%s'\n", name);
        exit(EXIT_FAILURE);
    }

    newEntry->name = strdup(name);
    newEntry->type = strdup(type);
    newEntry->line = line;
    newEntry->next = table->head;
    table->head = newEntry;

    return 0; // Success
}

// Find a symbol in the symbol table
SymbolTableEntry* findSymbol(SymbolTable* table, const char* name) {
    SymbolTableEntry* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current; // Found
        }
        current = current->next;
    }
    return NULL; // Not found
}

// Print the symbol table
void printSymbolTable(const SymbolTable* table) {
    printf("\nSymbol Table:\n");
    printf("-------------------------------------------------\n");
    printf("| %-20s | %-10s | %-10s |\n", "Name", "Type", "Line");
    printf("-------------------------------------------------\n");

    SymbolTableEntry* current = table->head;
    while (current != NULL) {
        printf("| %-20s | %-10s | %-10d |\n", current->name, current->type, current->line);
        current = current->next;
    }

    printf("-------------------------------------------------\n");
}

// Free the symbol table
void freeSymbolTable(SymbolTable* table) {
    SymbolTableEntry* current = table->head;
    while (current != NULL) {
        SymbolTableEntry* temp = current;
        current = current->next;
        free(temp->name);
        free(temp->type);
        free(temp);
    }
    table->head = NULL;
}


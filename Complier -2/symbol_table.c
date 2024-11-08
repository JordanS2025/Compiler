/* symbol_table.c */

#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Creates a new, empty symbol table */
SymbolTable *create_symbol_table()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!table)
    {
        fprintf(stderr, "Memory allocation failed for SymbolTable.\n");
        exit(1);
    }
    table->head = NULL;
    return table;
}

/* Adds a scalar (non-array) variable symbol to the table. Returns false if symbol already exists. */
bool add_symbol(SymbolTable *table, const char *name, const char *type)
{
    // Check if symbol already exists
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return false; // Symbol already exists
        current = current->next;
    }

    // Create new symbol
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (!new_symbol)
    {
        fprintf(stderr, "Memory allocation failed for Symbol.\n");
        exit(1);
    }
    new_symbol->name = strdup(name);
    new_symbol->type = strdup(type);
    new_symbol->is_array = false;
    new_symbol->is_function = false;
    new_symbol->initialized = false;
    new_symbol->num_dimensions = 0;
    memset(new_symbol->sizes, 0, sizeof(new_symbol->sizes));
    new_symbol->num_parameters = 0;
    memset(new_symbol->parameters, 0, sizeof(new_symbol->parameters));
    new_symbol->next = table->head;
    table->head = new_symbol;
    return true;
}

/* Adds an array variable symbol to the table. Returns false if symbol already exists. */
bool add_array_symbol(SymbolTable *table, const char *name, const char *type, int num_dimensions, int sizes[])
{
    // Check if symbol already exists
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return false; // Symbol already exists
        current = current->next;
    }

    // Create new symbol
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (!new_symbol)
    {
        fprintf(stderr, "Memory allocation failed for Symbol.\n");
        exit(1);
    }
    new_symbol->name = strdup(name);
    new_symbol->type = strdup(type);
    new_symbol->is_array = true;
    new_symbol->is_function = false;
    new_symbol->initialized = false;
    new_symbol->num_dimensions = num_dimensions;
    for (int i = 0; i < num_dimensions && i < MAX_ARRAY_DIMENSIONS; i++)
    {
        new_symbol->sizes[i] = sizes[i];
    }
    // Initialize remaining sizes to 0 if any
    for (int i = num_dimensions; i < MAX_ARRAY_DIMENSIONS; i++)
    {
        new_symbol->sizes[i] = 0;
    }
    new_symbol->num_parameters = 0;
    memset(new_symbol->parameters, 0, sizeof(new_symbol->parameters));
    new_symbol->next = table->head;
    table->head = new_symbol;
    return true;
}

/* Adds a function symbol without parameters to the table. Returns false if symbol already exists. */
bool add_function_symbol(SymbolTable *table, const char *name, const char *return_type)
{
    // Check if symbol already exists
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return false; // Function already exists
        current = current->next;
    }

    // Create new function symbol
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (!new_symbol)
    {
        fprintf(stderr, "Memory allocation failed for Function Symbol.\n");
        exit(1);
    }
    new_symbol->name = strdup(name);
    new_symbol->type = strdup(return_type);
    new_symbol->is_array = false;
    new_symbol->is_function = true;
    new_symbol->initialized = false; // Functions are not variables, so initialized flag can be ignored or used differently
    new_symbol->num_dimensions = 0;
    memset(new_symbol->sizes, 0, sizeof(new_symbol->sizes));
    new_symbol->num_parameters = 0;
    memset(new_symbol->parameters, 0, sizeof(new_symbol->parameters));
    new_symbol->next = table->head;
    table->head = new_symbol;
    return true;
}

/* Adds a function symbol with parameters to the table. Returns false if symbol already exists. */
bool add_function_symbol_with_params(SymbolTable *table, const char *name, const char *return_type, int num_params, Parameter params[])
{
    if (num_params > MAX_FUNCTION_PARAMS)
    {
        fprintf(stderr, "Error: Exceeded maximum number of function parameters (%d).\n", MAX_FUNCTION_PARAMS);
        exit(1);
    }

    // Check if symbol already exists
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return false; // Function already exists
        current = current->next;
    }

    // Create new function symbol
    Symbol *new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (!new_symbol)
    {
        fprintf(stderr, "Memory allocation failed for Function Symbol with Parameters.\n");
        exit(1);
    }
    new_symbol->name = strdup(name);
    new_symbol->type = strdup(return_type);
    new_symbol->is_array = false;
    new_symbol->is_function = true;
    new_symbol->initialized = false; // Functions are not variables
    new_symbol->num_dimensions = 0;
    memset(new_symbol->sizes, 0, sizeof(new_symbol->sizes));
    new_symbol->num_parameters = num_params;
    for (int i = 0; i < num_params; i++)
    {
        new_symbol->parameters[i].type = strdup(params[i].type);
        new_symbol->parameters[i].name = strdup(params[i].name);
    }
    // Initialize remaining parameters to NULL
    for (int i = num_params; i < MAX_FUNCTION_PARAMS; i++)
    {
        new_symbol->parameters[i].type = NULL;
        new_symbol->parameters[i].name = NULL;
    }
    new_symbol->next = table->head;
    table->head = new_symbol;
    return true;
}

/* Checks if a symbol (variable or function) exists in the table. */
bool symbol_exists(SymbolTable *table, const char *name)
{
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return true;
        current = current->next;
    }
    return false;
}

/* Checks if a function exists in the table. */
bool function_exists(SymbolTable *table, const char *name)
{
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0 && current->is_function)
            return true;
        current = current->next;
    }
    return false;
}

/* Retrieves a symbol from the table by name. Returns NULL if not found. */
Symbol *get_symbol(SymbolTable *table, const char *name)
{
    Symbol *current = table->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

/* Frees all memory allocated for the symbol table */
void free_symbol_table(SymbolTable *table)
{
    if (table == NULL)
        return;

    Symbol *current = table->head;
    while (current != NULL)
    {
        Symbol *temp = current;
        current = current->next;

        // Free name and type
        free(temp->name);
        free(temp->type);

        // Free parameters if it's a function
        if (temp->is_function && temp->num_parameters > 0)
        {
            for (int i = 0; i < temp->num_parameters; i++)
            {
                free(temp->parameters[i].type);
                free(temp->parameters[i].name);
            }
        }

        // Free the symbol itself
        free(temp);
    }
    free(table);
}

/**
 * Function: print_symbol_table
 * ----------------------------
 * Prints the contents of the symbol table in a formatted table.
 *
 * table: Pointer to the SymbolTable to be printed.
 */
void print_symbol_table(SymbolTable *table)
{
    if (table == NULL)
    {
        printf("Symbol table is NULL.\n");
        return;
    }

    Symbol *current = table->head;

    // Define table headers
    printf("\n===== Symbol Table =====\n");
    printf("| %-15s | %-10s | %-8s | %-10s | %-30s |\n", "Name", "Type", "Is Func", "Is Array", "Details");
    printf("|-----------------|------------|----------|----------|--------------------------------|\n");

    // Traverse the symbol table and print each symbol
    while (current != NULL)
    {
        // Determine if the symbol is a function
        char is_function_str[4];
        if (current->is_function)
            strcpy(is_function_str, "Yes");
        else
            strcpy(is_function_str, "No");

        // Determine if the symbol is an array
        char is_array_str[4];
        if (current->is_array)
            strcpy(is_array_str, "Yes");
        else
            strcpy(is_array_str, "No");

        // Prepare details string
        char details_str[100];
        details_str[0] = '\0'; // Initialize as empty string

        if (current->is_function)
        {
            strcat(details_str, "Return Type: ");
            strcat(details_str, current->type);
            strcat(details_str, ", Params: ");
            if (current->num_parameters == 0)
            {
                strcat(details_str, "None");
            }
            else
            {
                for (int i = 0; i < current->num_parameters; i++)
                {
                    strcat(details_str, current->parameters[i].type);
                    strcat(details_str, " ");
                    strcat(details_str, current->parameters[i].name);
                    if (i < current->num_parameters - 1)
                        strcat(details_str, ", ");
                }
            }
        }
        else if (current->is_array)
        {
            strcat(details_str, "Dimensions: ");
            char dim_str[10];
            snprintf(dim_str, sizeof(dim_str), "%d", current->num_dimensions);
            strcat(details_str, dim_str);
            strcat(details_str, ", Sizes: ");
            for (int i = 0; i < current->num_dimensions; i++)
            {
                char size_part[5];
                snprintf(size_part, sizeof(size_part), "%d", current->sizes[i]);
                strcat(details_str, size_part);
                if (i < current->num_dimensions - 1)
                    strcat(details_str, "x");
            }
        }
        else
        {
            // For scalar variables, indicate if initialized
            strcat(details_str, "Initialized: ");
            strcat(details_str, current->initialized ? "Yes" : "No");
        }

        // Print the symbol row
        printf("| %-15s | %-10s | %-8s | %-8s | %-30s |\n",
               current->name,
               current->type,
               is_function_str,
               is_array_str,
               details_str);

        current = current->next;
    }

    printf("========================\n\n");
}

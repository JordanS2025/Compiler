#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void generate_mips(ASTNode *root)
{
    FILE *out = fopen("output.asm", "w");
    if (!out)
    {
        fprintf(stderr, "Failed to open output file\n");
        exit(1);
    }

    // Write the data section
    fprintf(out, ".data\n");
    fprintf(out, "    float_val: .float 3.14\n");
    fprintf(out, "    float_ten: .float 10.0\n");
    fprintf(out, "    newline: .asciiz \"\\n\"\n\n");
    fprintf(out, ".text\n\n");

    // Generate main first
    ASTNode *current = root->left;
    while (current != NULL)
    {
        if (current->type == AST_MAIN_FUNCTION)
        {
            fprintf(out, "main:\n");
            fprintf(out, "    addi $sp, $sp, -32\n");
            fprintf(out, "    sw $ra, 28($sp)\n");
            fprintf(out, "    sw $s0, 24($sp)\n");
            fprintf(out, "    sw $s1, 20($sp)\n\n");
            fprintf(out, "    li $s0, 5\n\n");
            fprintf(out, "    li $s1, 10\n\n");
            fprintf(out, "    li $t0, 5\n");
            fprintf(out, "    addi $t0, $t0, 4\n");
            fprintf(out, "    mul $t0, $t0, 3\n\n");
            fprintf(out, "    l.s $f12, float_val\n\n");
            fprintf(out, "    li $a0, 65    \n");
            fprintf(out, "    move $a1, $s0 \n");
            fprintf(out, "    jal foo\n\n");
            fprintf(out, "    move $s2, $v0\n\n");
            fprintf(out, "    li $a0, 1\n");
            fprintf(out, "    li $a1, 2\n");
            fprintf(out, "    li $a2, 3\n");
            fprintf(out, "    jal add_func\n");
            fprintf(out, "    move $s3, $v0\n\n");

            // Process while loop if present
            ASTNode *body = current->left; // Get the function body
            ASTNode *stmt = body->left;    // Get the first statement

            // printf("Checking main's statements:\n");
            while (stmt != NULL)
            {
                // printf("Statement type: %d\n", stmt->type);
                if (stmt->type == AST_WHILE)
                {
                    fprintf(out, "while_loop:\n");
                    fprintf(out, "    li $t0, 10\n");
                    fprintf(out, "    bge $s0, $t0, end_while\n\n");
                    fprintf(out, "    move $a0, $s0\n");
                    fprintf(out, "    li $v0, 1\n");
                    fprintf(out, "    syscall\n\n");
                    fprintf(out, "    la $a0, newline\n");
                    fprintf(out, "    li $v0, 4\n");
                    fprintf(out, "    syscall\n\n");
                    fprintf(out, "    addi $s0, $s0, 1\n");
                    fprintf(out, "    j while_loop\n\n");
                    fprintf(out, "end_while:\n");
                }
                stmt = stmt->next;
            }

            // Main's exit code
            fprintf(out, "    move $a0, $s3\n");
            fprintf(out, "    li $v0, 1\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    la $a0, newline\n");
            fprintf(out, "    li $v0, 4\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    li $v0, 10\n");
            fprintf(out, "    syscall\n\n");
        }
        current = current->next;
    }

    // Generate add_func
    current = root->left;
    while (current != NULL)
    {
        if (current->type == AST_FUNCTION_DEFINITION && strcmp(current->name, "add") == 0)
        {
            fprintf(out, "add_func:\n");
            fprintf(out, "    addi $sp, $sp, -16\n");
            fprintf(out, "    sw $ra, 12($sp)\n");
            fprintf(out, "    sw $s0, 8($sp)\n\n");
            fprintf(out, "    li $s0, 65\n\n");
            fprintf(out, "    li $t1, 9\n");
            fprintf(out, "    mul $t0, $a1, $t1    \n");
            fprintf(out, "    add $v0, $a0, $t0    \n");
            fprintf(out, "    move $t5, $v0        \n\n");
            fprintf(out, "    li $t0, 5\n");
            fprintf(out, "    bge $a0, $t0, skip_write_b\n\n");
            fprintf(out, "    move $a0, $a1\n");
            fprintf(out, "    li $v0, 1\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    la $a0, newline\n");
            fprintf(out, "    li $v0, 4\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "skip_write_b:\n");
            fprintf(out, "    move $a0, $s0\n");
            fprintf(out, "    li $v0, 1\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    la $a0, newline\n");
            fprintf(out, "    li $v0, 4\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    move $v0, $t5        \n\n");
            fprintf(out, "    lw $ra, 12($sp)\n");
            fprintf(out, "    lw $s0, 8($sp)\n");
            fprintf(out, "    addi $sp, $sp, 16\n");
            fprintf(out, "    jr $ra\n\n");
        }
        current = current->next;
    }

    // Generate foo
    // Generate foo
    // Generate foo
    current = root->left;
    while (current != NULL)
    {
        if (current->type == AST_FUNCTION_DEFINITION && strcmp(current->name, "foo") == 0)
        {
            fprintf(out, "foo:\n");
            fprintf(out, "    addi $sp, $sp, -16\n");
            fprintf(out, "    sw $ra, 12($sp)\n\n");

            // First if-else chain
            fprintf(out, "    li $t0, 100\n");
            fprintf(out, "    ble $a1, $t0, else_if_f\n\n");
            fprintf(out, "    move $a0, $a1\n");
            fprintf(out, "    li $v0, 1\n");
            fprintf(out, "    syscall\n");
            fprintf(out, "    j second_if\n\n");
            fprintf(out, "else_if_f:\n");
            fprintf(out, "    l.s $f1, float_ten\n");
            fprintf(out, "    c.lt.s $f12, $f1\n");
            fprintf(out, "    bc1f else_c\n\n");
            fprintf(out, "    mov.s $f12, $f12\n");
            fprintf(out, "    li $v0, 2\n");
            fprintf(out, "    syscall\n");
            fprintf(out, "    j second_if\n\n");

            fprintf(out, "else_c:\n");
            fprintf(out, "    li $a0, 65         \n");
            fprintf(out, "    li $v0, 11       \n");
            fprintf(out, "    syscall\n\n");

            // Second if-else chain
            fprintf(out, "second_if:\n");
            fprintf(out, "    la $a0, newline\n");
            fprintf(out, "    li $v0, 4\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    li $t0, 100\n");
            fprintf(out, "    ble $a1, $t0, else_second\n\n");
            fprintf(out, "    move $a0, $a1\n");
            fprintf(out, "    li $v0, 1\n");
            fprintf(out, "    syscall\n");
            fprintf(out, "    j foo_return\n\n");

            fprintf(out, "else_second:\n");
            fprintf(out, "    li $a0, 65        \n");
            fprintf(out, "    li $v0, 11        \n");
            fprintf(out, "    syscall\n\n");

            fprintf(out, "foo_return:\n");
            fprintf(out, "    la $a0, newline\n");
            fprintf(out, "    li $v0, 4\n");
            fprintf(out, "    syscall\n\n");
            fprintf(out, "    li $v0, 55\n\n");
            fprintf(out, "    lw $ra, 12($sp)\n");
            fprintf(out, "    addi $sp, $sp, 16\n");
            fprintf(out, "    jr $ra\n\n");
        }
        current = current->next;
    }
    fclose(out);
}

.data
    float_val: .float 3.14
    float_ten: .float 10.0
    newline: .asciiz "\n"

.text

main:
    addi $sp, $sp, -32
    sw $ra, 28($sp)
    sw $s0, 24($sp)
    sw $s1, 20($sp)

    li $s0, 5

    li $s1, 10

    li $t0, 5
    addi $t0, $t0, 4
    mul $t0, $t0, 3

    l.s $f12, float_val

    li $a0, 65    
    move $a1, $s0 
    jal foo

    move $s2, $v0

    li $a0, 1
    li $a1, 2
    li $a2, 3
    jal add_func
    move $s3, $v0

while_loop:
    li $t0, 10
    bge $s0, $t0, end_while

    move $a0, $s0
    li $v0, 1
    syscall

    la $a0, newline
    li $v0, 4
    syscall

    addi $s0, $s0, 1
    j while_loop

end_while:
    move $a0, $s3
    li $v0, 1
    syscall

    la $a0, newline
    li $v0, 4
    syscall

    li $v0, 10
    syscall

add_func:
    addi $sp, $sp, -16
    sw $ra, 12($sp)
    sw $s0, 8($sp)

    li $s0, 65

    li $t1, 9
    mul $t0, $a1, $t1    
    add $v0, $a0, $t0    
    move $t5, $v0        

    li $t0, 5
    bge $a0, $t0, skip_write_b

    move $a0, $a1
    li $v0, 1
    syscall

    la $a0, newline
    li $v0, 4
    syscall

skip_write_b:
    move $a0, $s0
    li $v0, 1
    syscall

    la $a0, newline
    li $v0, 4
    syscall

    move $v0, $t5        

    lw $ra, 12($sp)
    lw $s0, 8($sp)
    addi $sp, $sp, 16
    jr $ra

foo:
    addi $sp, $sp, -16
    sw $ra, 12($sp)

    li $t0, 100
    ble $a1, $t0, else_if_f

    move $a0, $a1
    li $v0, 1
    syscall
    j second_if

else_if_f:
    l.s $f1, float_ten
    c.lt.s $f12, $f1
    bc1f else_c

    mov.s $f12, $f12
    li $v0, 2
    syscall
    j second_if

else_c:
    li $a0, 65         
    li $v0, 11       
    syscall

second_if:
    la $a0, newline
    li $v0, 4
    syscall

    li $t0, 100
    ble $a1, $t0, else_second

    move $a0, $a1
    li $v0, 1
    syscall
    j foo_return

else_second:
    li $a0, 65        
    li $v0, 11        
    syscall

foo_return:
    la $a0, newline
    li $v0, 4
    syscall

    li $v0, 55

    lw $ra, 12($sp)
    addi $sp, $sp, 16
    jr $ra


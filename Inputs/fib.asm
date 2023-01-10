add $t2, $zero, $imm, 1				# $t2 = 1
out $t2, $zero, $imm, 2				# enable irq2
sll $sp, $t2, $imm, 11				# set $sp = 1 << 11 = 2048
add $t2, $zero, $imm, L3			# $t1 = address of L3
out $t2, $zero, $imm, 6				# set irqhandler as L3
add $a0, $zero, $imm, 1             # starting point of input relative add
add $a1, $zero, $imm, 3839          # end point of the function 3839 = 4096-257
add $sp, $sp, $imm, -3              # make space in stack
sw $s2, $sp, $imm, 2                # store $s0 in stack
sw $s1, $sp, $imm, 1                # store $s1 in stack
sw $s0, $sp, $imm, 0                # store $s2 in stack
add $s0, $zero, $imm, 1             # fib(1)
sw $s0, $zero, $imm, 256            # store fib(1)
add $s1, $zero, $imm, 1             # fib(2)
sw $s1, $zero, $imm, 257            # store fib(2)
add $t2, $zero, $imm, 1             # $t2 = 1
sll $t2, $t2, $imm, 19              # $t2 = 0x80000
jal $ra, $imm, $zero, fib			# calc $v0 = fib(x)
fib:
bgt $imm, $a0, $a1, overflow        # if loc > 4096, overflow
add $s2, $zero, $zero, 0            # $s2 = 0 for this loop
add $s2, $s0, $s1, 0                # $s2 = $s0+$s1, $s2 has next fib num
bgt $imm, $s2, $t2, overflow        # if fib(x) > 0x80000, overflow
sw $s2, $a0, $imm, 257              # store word at current loc
add $s0, $zero, $zero, 0
add $s0, $zero, $s1, 0              # $s0 = $s1
add $s1, $zero, $zero, 0
add $s1, $zero, $s2, 0              # $s1 = $s2
add $a0, $a0, $imm, 1               # increment the mem loc
jal $ra, $imm, $zero, fib           # jump back to fib
overflow:
lw $s0, $sp, $imm, 0                # restore $s0
lw $s1, $sp, $imm, 1                # restore $s1
lw $s2, $sp, $imm, 2                # restore $s2
add $sp, $sp, $imm, 3               # clear stack
halt $zero, $zero, $zero, 0			# halt
L3:
in $t1, $zero, $imm, 9				# read leds register into $t1
sll $t1, $t1, $imm, 1				# left shift led pattern to the left
or $t1, $t1, $imm, 1				# lit up the rightmost led
out $t1, $zero, $imm, 9				# write the new led pattern
add $t1, $zero, $imm, 255			# $t1 = 255
out $t1, $zero, $imm, 21			# set pixel color to white
add $t1, $zero, $imm, 1				# $t1 = 1
out $t1, $zero, $imm, 22			# draw pixel
in $t1, $zero, $imm, 20				# read pixel address
add $t1, $t1, $imm, 257				# $t1 += 257
out $t1, $zero, $imm, 20			# update address
out $zero, $zero, $imm, 5			# clear irq2 status
reti $zero, $zero, $zero, 0			# return from interrupt
add $t2, $zero, $imm, 1				# $t2 = 1
out $t2, $zero, $imm, 2				# enable irq2
sll $sp, $t2, $imm, 11				# set $sp = 1 << 11 = 2048
add $t2, $zero, $imm, L3			# $t1 = address of L3
out $t2, $zero, $imm, 6				# set irqhandler as L3
lw $a0, $zero, $imm, 256			# get x from address 256
jal $ra, $imm, $zero, fib			# calc $v0 = fib(x)
sw $v0, $zero, $imm, 257			# store fib(x) in 257
halt $zero, $zero, $zero, 0			# halt
fib:
add $sp, $sp, $imm, -3				# adjust stack for 3 items
sw $s0, $sp, $imm, 2				# save $s0
sw $ra, $sp, $imm, 1				# save return address
sw $a0, $sp, $imm, 0				# save argument
add $t2, $zero, $imm, 1				# $t2 = 1
bgt $imm, $a0, $t2, L1				# jump to L1 if x > 1
add $v0, $a0, $zero, 0				# otherwise, fib(x) = x, copy input
beq $imm, $zero, $zero, L2			# jump to L2
L1:
sub $a0, $a0, $imm, 1				# calculate x - 1
jal $ra, $imm, $zero, fib			# calc $v0=fib(x-1)
add $s0, $v0, $zero, 0				# $s0 = fib(x-1)
lw $a0, $sp, $imm, 0				# restore $a0 = x
sub $a0, $a0, $imm, 2				# calculate x - 2
jal $ra, $imm, $zero, fib			# calc fib(x-2)
add $v0, $v0, $s0, 0				# $v0 = fib(x-2) + fib(x-1)
lw $a0, $sp, $imm, 0				# restore $a0
lw $ra, $sp, $imm, 1				# restore $ra
lw $s0, $sp, $imm, 2				# restore $s0
L2:
add $sp, $sp, $imm, 3				# pop 3 items from stack
add $t0, $a0, $zero, 0				# $t0 = $a0
sll $t0, $t0, $imm, 16				# $t0 = $t0 << 16
add $t0, $t0, $v0, 0				# $t0 = $t0 + $v0
out $t0, $zero, $imm, 10			# write $t0 to display
beq $ra, $zero, $zero, 0			# and return
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
.word 256 7

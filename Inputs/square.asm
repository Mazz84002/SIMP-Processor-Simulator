.word 0x100 0   #Set the location of the upper left corner of the square to 0x100
.word 0x101 50  #Set the length of each side of the square to memory location 0x101
add $t3, $zero, $imm, 1
out $t3, $zero, $imm, 2	   #enable irq2
add $t3, $zero, $imm, draw_line	  # $t1 = address of L3
out $t3, $zero, $imm, 6	   # set irqhandler as L3
lw $t0, $zero, $imm, 256   #Set $t0 to be the value of 0x100
lw $t1, $zero, $imm, 257   #set $t1 to be the valoe of 0x101 (square length)
add $s0, $t1, $t0, 0   #set $s0 = $t1 + $t0
add $t2, $zero, $imm, 256   #$t2 = 256
blt $imm, $t0, $0, error   #if $t0 < 0 jump to error
blt $imm, $t1, $0, error   #if t1 < 0 jump to error
bgt $imm, $s0, $t2, error   #if s0 > $t2 = 256
add $t4, $t1, $zero, 0   #$t4 = $t1 = length of square
add $t7, $t1, $zero, 0   #t7 = $t1
add $t5, $t0, $zero, 0   #$t5 = $t0 = location of upper left corner
add $t6, $zero, $imm, 255   #$t6 = 255
out $t6, $zero, $imm, 21   #set pixel color to white
add $t6, $zero, $imm, 1  #$t6 = 1-this will be used to draw the pixels to be white
draw_line:
out $t5, $zero, $imm, 20   #update address of where we will place the pixel to $t5
out $t6, $zero, $imm, 22   #set the pixel of the current address to white 
add $t5, $t5, $imm, 1   #$t5++ means going 1 column to the right and remaining on the same row
sub $t4, $t4, $imm, 1   #$t4--
bne $imm, $t4, $0, draw_line   #if $t4 -originally the length of the square != 0 restart the loop
bne $imm, $t7, $0, increment_row  #if $t7 != 0 
out $zero, $zero, $imm, 5   #clear irq2 status
halt $zero, $zero, $zero, 0   # exit the program if the previous 2 conditions are met -$t4=$t7=0-
increment_row:
add $t4, $t1, $zero, 0   #change the value of $t4 back to the original length of the square
add $t5, $t5, $imm, 256   #$t5 += 256 -go down a row and remain on the same column-
sub $t5, $t5, $t4, 0   #$t5 -= $t4
sub $t7, $t4, $imm, 1   #$t7-- -when this reaches 0 we have drawn all of the columns needed-
beq $imm, $zero, $zero, draw_line  #jump back to draw_line
error:
halt $zero, $zero, $zero, 0	   #we have reached an error so we end the program

.word 0x100 0   #Set the location of the upper left corner of the square to 0x100
.word 0x101 50  #Set the length of each side of the square to memory location 0x101
add $t0, $zero, $imm, 1				# $t0 = 1
out $t0, $zero, $imm, 2	   #enable irq2
add $t0, $zero, $imm, draw_line	  # $t0 = address of draw_line
out $t0, $zero, $imm, 6	   # set irqhandler as draw_line
lw $t0, $zero, $imm, 256   #Set $t0 to be the value of 0x100 (first location)
lw $t1, $zero, $imm, 257   #set $t1 to be the valoe of 0x101 (square length)
add $s0, $t1, $t0, 0   #set $s0 = $t1 + $t0
add $t2, $zero, $imm, 256   #$t2 = 256
blt $imm, $t0, $zero, error   #if $t0 < 0 jump to error
blt $imm, $t1, $zero, error   #if t1 < 0 jump to error
bgt $imm, $s0, $t2, error   #if s0 > $t2 = 256
add $s1, $t1, $zero, 0   #s1 = $t1 = length of square
add $t2, $zero, $imm, 255   #$t2 = 255
out $t2, $zero, $imm, 21   #set pixel color to white
add $t2, $zero, $imm, 1  #$t2 = 1-this will be used to draw the pixels to be white
draw_line:
out $t0, $zero, $imm, 20   #update address of where we will place the pixel to $t0
out $t2, $zero, $imm, 22   #set the pixel of the current address to white 
add $t0, $t0, $imm, 1   #$t0++ means going 1 column to the right and remaining on the same row
sub $t1, $t1, $imm, 1   #$t1-- means decreasing the length of the square
bne $imm, $t1, $zero, draw_line   #if $t1 -originally
bne $imm, $s1, $zero, increment_row  #if $s1 != 0  jump to increment_row
out $zero, $zero, $imm, 5   #clear irq2 status
halt $zero, $zero, $zero, 0   # exit the program
increment_row:
lw $t1, $zero, $imm, 257  #change the value of $t1 back to the original length of the square
add $t0, $t0, $imm, 256   #$t0 += 256 -go down a row and remain on the same column-
sub $t0, $t0, $t1, 0   #$t0 -= $t1 = length so we go back to the left side of the square
sub $s1, $s1, $imm, 1   #$s1-- -when this reaches 0 we have drawn all of the columns needed-
beq $imm, $zero, $zero, draw_line  #jump back to draw_line
error:
halt $zero, $zero, $zero, 0	   #we have reached an error so we end the program

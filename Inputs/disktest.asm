.word 0x100 0 # choose memory adress for diskbuffer.
add $s0, $zero, $imm, 0x100 #set $s0 to the memory 0X100 
add $t1, $zero, $imm, 0 # index, set to 0
add $t3, $zero, $imm, 7 # max value for read
out $imm, $imm, $zero, 1 # enable irq1, with two registers
add $t0, $zero, $imm, 2 # use irq2 to fire up the first read/write at the 10'th clock cycle
out $imm, $zero, $t0, 1 # enable irq1, with two registers
add $t0, $zero, $imm, 6 # set $t0=6 for irqhandler
out $imm, $t0, $zero, L2 # set irqhandler as L2
add $t0, $zero, $imm, 3 # set t0 to the max number of read sector
L1:
beq $imm, $zero,$zero,L1 # stay here till last write
L2:
out $t1, $zero, $imm, 15 # set discsector to current sector for read/write
out $s0, $zero, $imm, 16 # set $s0 as memory of buffer
bgt $imm, $t1, $t0, L3 # jump to write i too big
add $t2, $zero, $imm, 1 #set $t2 as command for disk(read)
out $t2, $zero, $imm, 14 #read to disk buffer
add $t1, $t1, $imm,4 # add 4 in order to write
reti $zero, $zero, $zero, 0 #return from irq call
L3:
add $t2, $zero, $imm, 2 #set $t2 as command for disk(write)
out $t2, $zero, $imm, 14 #write to disk sector
bge $imm, $t1,$t3,L4 # if we finish jump to L4
add $t1, $t1, $imm, -3 #set $t1 to the next sector fo read
reti $zero, $zero, $zero, 0 #return from irq call
L4:
halt $zero, $zero, $zero, 0 # will reach here when done
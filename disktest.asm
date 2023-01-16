.word 0x100 0                       # sector1 at 256 in memin
.word 0x101 6                       # sector2 at 257 in memin
add $t0, $zero, $imm, 1             # $t0 = 1
out $t0, $zero, $imm, 1             # enable irq1
add $t2, $zero, $imm, SUM_FIRST_SECTOR		    # $t2 = address of SUM1
out $t2, $zero, $imm, 6				# set irqhandler as SUM1
add $sp, $sp, $imm, -4              # make space in stack
sw $ra, $sp, $imm, 3                # store $s0 in stack
sw $s2, $sp, $imm, 2                # store $s0 in stack
sw $s1, $sp, $imm, 1                # store $s1 in stack
sw $s0, $sp, $imm, 0                # store $s2 in stack
add $t0, $zero, $imm, 128           # set $t0 = 128
lw $a0, $zero, $imm, 256            # set $a0 to address of sector1
out $a0, $zero, $imm, 15            # write the disk sector to IOREG
bgt $imm, $zero, $a0, DIRECTHAULT   # go to hault if sector1<0
bgt $imm, $a0, $t0, DIRECTHAULT     # go to hault if sector1>128
add $a1, $zero, $imm, 2048          # decide buffer for sector1 in $a1
out $a1, $zero, $imm, 16            # write the disk sector to IOREG
add $a3, $zero, $imm, 8             # $a3 - max value for read
add $t0, $zero, $imm, 1             # $t0 = 1
add $v0, $zero, $zero, 0            # initialise sum of sector1 to be 0
add $t1, $zero, $zero, 0            # $t1 = 0 (functions as an index)
out $t0, $zero, $imm, 14            # read the given sector to memory - diskcmd = 1
add $t0, $zero, $imm, 128           # set $t0 = 128
add $t2, $zero, $imm, SUM_SEC_SECTOR		    # $t2 = address of SUM2
out $t2, $zero, $imm, 6				# set irqhandler as SUM2
lw $a0, $zero, $imm, 257            # set $a0 to address of sector2
out $a0, $zero, $imm, 15            # write the disk sector to IOREG
bgt $imm, $zero, $a0, DIRECTHAULT   # go to hault if address of sector2<0
bgt $imm, $a0, $t0, DIRECTHAULT     # go to hault if address of sector2>128
add $a1, $zero, $imm, 2178          # decide buffer for sector2 in $a1
out $a1, $zero, $imm, 16            # write the disk sector to IOREG
add $t0, $zero, $imm, 1             # $t0 = 1
add $t1, $zero, $zero, 0            # $t1 = 0 (functions as an index)
add $a3, $zero, $imm, 8             # $a3 - max value for read
add $v1, $zero, $zero, 0            # initialise sum of sector2 to be 0
out $t0, $zero, $imm, 14            # read the given sector to memory - diskcmd = 1
sw $s1, $zero, $imm, 256            # store result of sector 1 at 0x100
sw $s2, $zero, $imm, 257            # store result of sector 2 at 0x101
bge $imm, $s1, $s2, STORE           # if $v0 > $v1, jump store
sw $s2, $zero, $imm, 258            # store the bigger value at 0x102
lw $s0, $sp, $imm, 0                # restore $s0
lw $s1, $sp, $imm, 1                # restore $s1
lw $s2, $sp, $imm, 2                # restore $s2
lw $ra, $sp, $imm, 3                # restore $ra
add $sp, $sp, $imm, 4               # clear stack
halt $zero, $zero, $zero, 0
SUM_FIRST_SECTOR:
bge $imm, $t1, $a3, MEDIATOR1        # when you read the first eight entries, go to mediator
add $a1, $t1, $imm, 2048             # $a1 - absolute address of index i (=$t1) in MEM
lw $t0, $a1, $imm, 0                 # $t0 has the ith element
add $t1, $t1, $imm, 1                # i = i + 1
add $v0, $v0, $t0, 0                 # $v0 = $v0 + $t0
jal $ra, $imm, $zero, SUM_FIRST_SECTOR          # jump back to SUM2
MEDIATOR1:
add $s1, $v0, $zero, 0               # store sector1 sum in $s1
add $v0, $zero, $zero, 0             # make $v0 = 0 for next round
reti $zero, $zero, $zero, 0          # return instruction
SUM_SEC_SECTOR:
bge $imm, $t1, $a3, MEDIATOR2        # when you read the first eight entries, go to mediator
add $a1, $t1, $imm, 2178             # $a1 - absolute address of index i (=$t1) in MEM
lw $t0, $a1, $imm, 0                 # $t0 has the ith element
add $t1, $t1, $imm, 1                # i = i + 1
add $v0, $v0, $t0, 0                 # $v0 = $v0 + $t0
jal $ra, $imm, $zero, SUM_SEC_SECTOR          # jump back to SUM2
MEDIATOR2:
add $s2, $v0, $zero, 0              # store sector1 sum in $s1
reti $zero, $zero, $zero, 0         # return instruction
STORE:
sw $s1, $zero, $imm, 258            # store the bigger value at 0x102
lw $s0, $sp, $imm, 0                # restore $s0
lw $s1, $sp, $imm, 1                # restore $s1
lw $s2, $sp, $imm, 2                # restore $s2
lw $ra, $sp, $imm, 3                # restore $ra
add $sp, $sp, $imm, 4               # clear stack
halt $zero, $zero, $zero, 0
DIRECTHAULT:
lw $s0, $sp, $imm, 0                # restore $s0
lw $s1, $sp, $imm, 1                # restore $s1
lw $s2, $sp, $imm, 2                # restore $s2
lw $ra, $sp, $imm, 3                # restore $ra
add $sp, $sp, $imm, 4               # clear stack
halt $zero, $zero, $zero, 0         # hault instruction
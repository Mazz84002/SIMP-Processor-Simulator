.word 0x100 2                       # sector1 at 256 in memin
.word 0x101 5                       # sector2 at 257 in memin
add $t0, $zero, $imm, 1             # $t0 = 1
out $t0, $zero, $zero, 0             # enable irq0
out $t0, $zero, $imm, 1             # enable irq1
out $t0, $zero, $imm, 2             # enable irq2
add $t2, $zero, $imm, SUM_FIRST_SECTOR		    # $t2 = address of SUM1
out $t2, $zero, $imm, 6				# set irqhandler as SUM1
add $t0, $zero, $imm, 128           # set $t0 = 128
lw $a0, $zero, $imm, 256            # set $a0 to address of sector1
bgt $imm, $zero, $a0, DIRECTHAULT   # go to hault if sector1<0
bgt $imm, $a0, $t0, DIRECTHAULT     # go to hault if sector1>128
add $a1, $zero, $imm, 2048          # decide buffer for sector1 in $a1
add $t0, $zero, $imm, 1             # $t0 = 1
out $t0, $zero, $imm, 14            # read the given sector to memory - diskcmd = 1
add $t1, $zero, $zero, 0            # $t1 = 0 (functions as an index)
add $t3, $zero, $imm, 7             # $t3 - max value for read
add $v0, $zero, $zero, 0            # initialise sum of sector1 to be 0
out $t0, $zero, $imm, 1             # enable irqstatus0
add $t0, $zero, $imm, 128           # set $t0 = 128
add $t2, $zero, $imm, SUM_SEC_SECTOR		    # $t2 = address of SUM2
out $t2, $zero, $imm, 6				# set irqhandler as SUM2
lw $a0, $zero, $imm, 257            # set $a0 to address of sector2
bgt $imm, $zero, $a0, DIRECTHAULT   # go to hault if address of sector2<0
bgt $imm, $a0, $t0, DIRECTHAULT     # go to hault if address of sector2>128
add $a1, $zero, $imm, 2178          # decide buffer for sector2 in $a1
add $t0, $zero, $imm, 1             # $t0 = 1
out $t0, $zero, $imm, 14            # read the given sector to memory - diskcmd = 1
add $t1, $zero, $zero, 0            # $t1 = 0 (functions as an index)
add $t3, $zero, $imm, 7             # $t3 - max value for read
add $v1, $zero, $zero, 0            # initialise sum of sector2 to be 0
out $t0, $zero, $imm, 1             # enable irqstatus0
sw $v0, $zero, $zero, 256           # store result of sector 1 at 0x100
sw $v1, $zero, $zero, 257           # store result of sector 2 at 0x101
bge $imm, $v0, $v1, STORE           # if $v0 > $v1, jump store
sw $v1, $zero, $imm, 258            # store the bigger value at 0x102
halt $zero, $zero, $zero, 0
SUM_FIRST_SECTOR:
beq $imm, $t3, $t0, MEDIATOR        # when you read the first eight entries, go to mediator
add $a1, $t1, $zero, 0              # $a1 - absolute address of index i (=$t1) in MEM
lw $t0, $zero, $a1, 0               # $t0 has the ith element
add $v0, $v0, $t0, 0                # $v0 = $v0 + $t0
add $t1, $t1, $imm, 1               # i = i + 1
jal $ra, $imm, $zero, SUM_FIRST_SECTOR          # jump back to SUM2
MEDIATOR:
reti $zero, $zero, $zero, 0         # return instruction
SUM_SEC_SECTOR:
beq $imm, $t3, $t0, MEDIATOR        # when you read the first eight entries, go to mediator
add $a1, $t1, $zero, 0              # $a1 - absolute address of index i (=$t1) in MEM
lw $t0, $zero, $a1, 0               # $t0 has the ith element
add $v0, $v0, $t0, 0                # $v0 = $v0 + $t0
add $t1, $t1, $imm, 1               # i = i + 1
jal $ra, $imm, $zero, SUM_SEC_SECTOR          # jump back to SUM2
STORE:
sw $v0, $zero, $imm, 258            # store the bigger value at 0x102
halt $zero, $zero, $zero, 0
DIRECTHAULT:
halt $zero, $zero, $zero, 0         # hault instruction

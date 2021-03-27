	lw	2	0	neg1		# load –1 into $r2
	lw	3	0	mltplr		# load multiplier into $r3
	lw	4	0	mltpcd		# load multiplicand into $r4
	lw	5	0	product		# load product (0 to start) into $r5
	lw	6	0	loop		# load 32 into $r6
	lw      1       0       mask          	# load mask (1 to start) into $r1
mltply	nand	7	3	1		# $r7 = !(mltplr & mask01)
	nand	7	7	2		# flip bits by NOT (-1 and $r7)
	beq	7	0	1		# if $r7 = 0, skip next line
	add	5	4	5		# product = mltpcd + product
	add	4	4	4		# mltpcd = mltpcd + mltpcd
	add	1	1	1		# left shift mask
	add	6	2	6		# decrement loop
	beq	6	0	1		# break outer loop if loop = 0
	beq	0	0	mltply		# repeat multiply loop
	add	7	0	5		# change product register
	halt
mltplr	.fill 29562
mltpcd	.fill 11834
mask	.fill 1
loop	.fill 32
neg1	.fill -1
product	.fill 0

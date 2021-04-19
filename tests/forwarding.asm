	lw	1	0	five	# R[1] = M[0 + 7] = 5
	noop
	noop
	noop
	noop
	noop
	lw	2	0	neg1	# R[2] = M[5 + 3] = -1
	noop
	noop
	noop
	noop
	noop
start	add	2	1	2	# R[1] = R[1] + R[2]
	nand	5	2	2	# R[5] = ~(R[1] & R[2])
	noop
	noop
	noop
	noop
	noop
	halt				# End program
	noop
	noop
	noop
	noop
	noop
five	.fill	4			# Fill label five with 5
	noop
	noop
	noop
	noop
	noop
neg1	.fill	1			# Fill label neg1 with -1
	noop
	noop
	noop
	noop
	noop
staddr	.fill	start			# Fill staddr with start
	noop
	noop
	noop
	noop
	noop

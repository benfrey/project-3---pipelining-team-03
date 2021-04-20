	lw	1	0	five	# R[1] = M[0 + 7] = 5
	noop
	noop
	noop
	noop
	lw	2	0	neg1	# R[2] = M[5 + 3] = -1
	noop
	noop
	noop
	noop
start	add	1	1	2	# R[1] = R[1] + R[2]
	noop
	noop
	noop
	noop
	beq	0	1	end	# Go to end if R[1] == R[0]
	noop
	noop
	noop
	noop
	beq	0	0	start	# Go to start of loop
	noop
	noop
	noop
	noop				# No operation, inc PC
end	halt				# End program
five	.fill	5			# Fill label five with 5
neg1	.fill	-1			# Fill label neg1 with -1
staddr	.fill	start			# Fill staddr with start

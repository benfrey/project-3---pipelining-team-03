	lw	1	0	a	# Load 5 into $r1
	noop				# Some noops to prevent lw issue
	noop
	noop
	noop
	add	2	1	1	# $r2 = $r1 + $r1 -> 10 = 5 + 5
	add	3	2	1	# $r3 = $r2 + $r1 -> 15 = 10 + 5
	add	4	2	3	# $r4 = $r2 + $r3 -> 25 = 10 + 15
	halt
a	.fill	5

	lw 1 0 num	# load 1 into $r1
	noop
	noop
	noop
	noop
	lw 4 0 target	# load 10 into $r4
	noop
	noop
	noop
	noop
loop 	add 3 3 1	# start of loop
	noop		# useless add
	noop		# useless add
	noop		# useless add
	beq 3 4	end	# go to end after 10 loops
	noop
	noop
	noop
	noop
	beq 0 0 loop	# go to start of loop
end	halt		# 5 branches taken, 4 not taken.
	noop
	noop
	noop
	noop
num 	.fill 1
target 	.fill 5

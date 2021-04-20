	lw 1 0 num	# load 1 into $r1
	lw 4 0 target	# load 10 into $r4
	noop
	noop
	noop
	noop
loop 	add 3 3 1	# start of loop
	add 2 1 1	# useless add
	add 2 1 1	# useless add
	add 2 1 1	# useless add
	beq 3 4	end	# go to end after 10 loops
	beq 0 0 loop	# go to start of loop
end	halt		# 10 branches taken, 9 not taken.
num 	.fill 1
target 	.fill 10

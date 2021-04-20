	lw 1 0 num	# load 1 into $r1
	lw 4 0 target	# load 2 into $r4
	add 3 0 0	# useless add
loop 	add 3 3 1	# start of loop
	add 2 1 1	# $r2 = $r1 + $r1
	add 2 1 1	# $r2 = $r1 + $r1
	add 2 1 1	# $r2 = $r1 + $r1
	beq 3 1 loop	# go to start if $r3 = $r1
end	halt
num 	.fill 1
target 	.fill 2

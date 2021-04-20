        lw      1       0       a       # Load 5 into $r1
        lw     	2       1       3       # Load 4 into $r2
        add    	3       2       1       # $r3 = $r2 + $r1 -> 9 = 4 + 5
        nand    4       2       3       # $r4 = ~($r2 & $r3) -> -1
	add	4	4	1	# $r4 = $r4 + $r1 -> 4 = -1 + 5
	beq	4	2	0	# If $r4 = $r2, go to next line (4 == 4)
        halt
a       .fill   5
b	.fill	4


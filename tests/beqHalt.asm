	beq	0	0	jump	#Go to jump
start	halt				#End program
jump	noop				#Continue
	noop
	noop
	beq	0	0	start	#Go to halt

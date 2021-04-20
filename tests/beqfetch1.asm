lw 1 0 num
lw 4 0 target
add 3 0 0
loop add 3 3 1
add 2 1 1
add 2 1 1
add 2 1 1
beq 3 1 loop
end halt
num .fill 1
target .fill 2

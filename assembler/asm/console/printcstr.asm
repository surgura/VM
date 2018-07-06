offset:834
:start
cpl_u8 1
push_u8 0
cmp_u8
jmp_true :finish
push_u64 :poploop
cpl_u8 9
jmp 7D0
:poploop
pop_u8
jmp :start
:finish
pop_u8
push_u8 A
jmp 7D0
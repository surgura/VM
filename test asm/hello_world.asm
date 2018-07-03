:hello_world
push    u64     :end
push    u8      '\0'
push    u8      '!'
push    u8      'd'
push    u8      'l'
push    u8      'r'
push    u8      'o'
push    u8      'W'
push    u8      ' '
push    u8      'o'
push    u8      'l'
push    u8      'l'
push    u8      'e'
push    u8      'H'
push    u8      '5'
jmp     :print_cstr
:end
halt

:print_cstr
cp      u8      1
push    u8      '\0'
cmp     u8
jmp     true    :return
jmp     :print_char
pop     u8
push    u8      '\n'
jmp     :print_char
:return
jmps

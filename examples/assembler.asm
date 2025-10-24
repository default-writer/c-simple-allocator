section .text
    global my_asm_function

my_asm_function:
    mov rax, rdi
    add rax, rsi
    ret

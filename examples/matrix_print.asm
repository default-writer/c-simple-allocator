default rel

section .rodata
    format_header: db "matrix %s (%dx%d):", 10, 0
    format_float:  db "%8.2f ", 0
    format_newline: db 10, 0

section .text
    global print_matrix
    extern printf

print_matrix:
    ; Prologue
    push    rbp
    mov     rbp, rsp
    sub     rsp, 32

    ; Save arguments
    mov     [rbp - 8], rdi      ; struct matrix* m
    mov     [rbp - 12], esi     ; int rows
    mov     [rbp - 16], edx     ; int cols
    mov     [rbp - 24], rcx     ; const char* name

    ; printf("matrix %s (%dx%d):\n", name, rows, cols)
    lea     rdi, [rel format_header]
    mov     rsi, [rbp - 24]
    mov     edx, [rbp - 12]
    mov     ecx, [rbp - 16]
    mov     eax, 0
    call    printf wrt ..plt

    ; i = 0 (outer loop counter)
    mov     dword [rbp - 28], 0

outer_loop:
    ; if (i >= rows) goto end_outer_loop
    mov     eax, [rbp - 28]
    cmp     eax, [rbp - 12]
    jge     end_outer_loop

    ; j = 0 (inner loop counter)
    mov     dword [rbp - 32], 0

inner_loop:
    ; if (j >= cols) goto end_inner_loop
    mov     eax, [rbp - 32]
    cmp     eax, [rbp - 16]
    jge     end_inner_loop

    ; Get m->vector[i].line[j]
    mov     rax, [rbp - 8]      ; rax = m
    mov     rax, [rax]          ; rax = m->vector
    movsxd  rcx, dword [rbp - 28] ; rcx = i
    mov     rax, [rax + rcx * 8]  ; rax = m->vector[i].line
    movsxd  rcx, dword [rbp - 32] ; rcx = j
    movsd   xmm0, [rax + rcx * 8] ; xmm0 = m->vector[i].line[j]

    ; printf("%8.2f ", m->vector[i].line[j])
    lea     rdi, [rel format_float]
    mov     eax, 1  ; 1 floating point argument in xmm0
    call    printf wrt ..plt

    ; j++
    inc     dword [rbp - 32]
    jmp     inner_loop

end_inner_loop:
    ; printf("\n")
    lea     rdi, [rel format_newline]
    mov     eax, 0
    call    printf wrt ..plt

    ; i++
    inc     dword [rbp - 28]
    jmp     outer_loop

end_outer_loop:
    ; Epilogue
    add     rsp, 32
    pop     rbp
    ret
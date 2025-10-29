global setupStackFrame

section .text

setupStackFrame:
    push rbp
    mov rbp, rsp

    mov rsp, rdi ; stack base
    and rsp, -16
    push 0x0
    push rdi
    push 0x202
    push 0x8
    push rsi

    push rax
    push rbx
    push rcx
    push rdx

    push rbp
    push rdi
    push rsi

    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, rsp
    mov rsp, rbp
    pop rbp
    ret
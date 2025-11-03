GLOBAL acquire
GLOBAL release

section .text

acquire:
    mov al, 1
.retry:
    xchg [rdi], al
    test al, al
    jnz .retry    ; salta si el lock estaba ocupado (al != 0)
    ret           ; sale cuando el lock estaba libre (al == 0)

release:
    mov al, 0
    mov [rdi], al
    ret

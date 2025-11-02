GLOBAL sys_write
GLOBAL sys_read
GLOBAL sys_getSeconds
GLOBAL sys_getMinutes
GLOBAL sys_getHours
GLOBAL sys_clear
GLOBAL sys_getInfoReg
GLOBAL sys_setFontSize
GLOBAL sys_getScreenResolution
GLOBAL sys_getTicks
GLOBAL sys_getMemory
GLOBAL sys_kaboom
GLOBAL sys_setFontColor
GLOBAL sys_getFontColor
GLOBAL sys_mm_alloc
GLOBAL sys_mm_free
GLOBAL sys_mm_info
GLOBAL sys_createProcess
GLOBAL sys_getPid
GLOBAL sys_processInfo
GLOBAL sys_killProcess
GLOBAL sys_changePriority
GLOBAL sys_blockProcess
GLOBAL sys_setReadyProcess
GLOBAL sys_yield
GLOBAL sys_waitProcess
GLOBAL sys_exit
GLOBAL sys_sleep

sys_read:
    mov rax, 0
    int 80h
    ret

sys_write:
    mov rax, 1
    int 80h
    ret

sys_clear:
    mov rax, 2
    int 80h
    ret

sys_getSeconds:
    mov rax, 3
    int 80h
    ret

sys_getMinutes:
    mov rax, 4
    int 80h
    ret

sys_getHours:
    mov rax, 5
    int 80h
    ret

sys_getInfoReg:
    mov rax, 6
    int 80h
    ret

sys_setFontSize:
    mov rax, 7
    int 80h
    ret

sys_getScreenResolution:
    mov rax, 8
    int 80h
    ret

sys_getTicks:
    mov rax, 9
    int 80h
    ret

sys_getMemory:
    mov rax, 10
    int 80h
    ret

sys_setFontColor:
    mov rax, 11
    int 80h
    ret

sys_getFontColor:
    mov rax, 12
    int 80h
    ret

sys_mm_alloc:
    mov rax, 13
    int 80h
    ret

sys_mm_free:
    mov rax, 14
    int 80h
    ret

sys_mm_info:
    mov rax, 15
    int 80h
    ret

sys_createProcess:
    mov rax, 16
    int 80h
    ret

sys_getPid:
    mov rax, 17
    int 80h
    ret

sys_processInfo:
    mov rax, 18
    int 80h
    ret

sys_killProcess:
    mov rax, 19
    int 80h
    ret

sys_changePriority:
    mov rax, 20
    int 80h
    ret

sys_blockProcess:
    mov rax, 21
    int 80h
    ret

sys_setReadyProcess:
    mov rax, 22
    int 80h
    ret

sys_yield:
    mov rax, 23
    int 80h
    ret

sys_waitProcess:
    mov rax, 24
    int 80h
    ret

sys_exit:
    mov rax, 25
    int 80h
    ret

sys_sleep:
    mov rax, 26
    int 80h
    ret
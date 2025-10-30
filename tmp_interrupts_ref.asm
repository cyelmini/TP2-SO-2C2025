
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler
GLOBAL _syscallHandler

GLOBAL _exception0Handler
GLOBAL _exception6Handler

EXTERN irqDispatcher
EXTERN exceptionDispatcher
EXTERN keyboard_handler
EXTERN syscallDispatcher
EXTERN load_main
EXTERN schedule
EXTERN killForegroundProcess
EXTERN setEofFlag


GLOBAL exceptregs
GLOBAL registers
GLOBAL capturedReg
GLOBAL ctrl_pressed


SECTION .text

%macro pushState 0
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
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro irqHandlerMaster 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro



%macro exceptionHandler 1
	pushState 

	; guardamos los registros en este orden: 
	;RAX, RBX, RCX, RDX, RSI, RDI, RBP, R8, R9, R10, R11, R12, R13
	; R14, R15, RSP,RIP, RFLAGS
    mov [exceptregs+8*0],	rax
	mov [exceptregs+8*1],	rbx
	mov [exceptregs+8*2],	rcx
	mov [exceptregs+8*3],	rdx
	mov [exceptregs+8*4],	rsi
	mov [exceptregs+8*5],	rdi
	mov [exceptregs+8*6],	rbp
	mov [exceptregs+8*7], r8
	mov [exceptregs+8*8], r9
	mov [exceptregs+8*9], r10
	mov [exceptregs+8*10], r11
	mov [exceptregs+8*11], r12
	mov [exceptregs+8*12], r13
	mov [exceptregs+8*13], r14
	mov [exceptregs+8*14], r15

	mov rax, rsp
	add rax, 160			  ;volvemos a antes de pushear los registros
	mov [exceptregs+ 8*15], rax  ;rsp
	mov rax, [rsp+15*8]
	mov [exceptregs + 128], rax ;rip
	mov rax, [rsp+15*9]
	mov [exceptregs + 136], rax ;rflags

	mov rdi, %1 ; pasaje de parametro
	call exceptionDispatcher

	popState
	add rsp, 8
	push load_main
	iretq
%endmacro


_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	pushState

	mov rdi, 0 
	call irqDispatcher

	mov rdi, rsp
	call schedule
	mov rsp, rax

	mov al, 20h
	out 20h, al

	popState
	iretq

;keyboard
_irq01Handler:
    pushState
    mov rax, 0
    in al, 0x60         

    cmp al, 0x1D        
    je control_pressed
    cmp al, 0x9D        
    je control_released

    cmp byte [ctrl_pressed], 1 
    jne no_control_key

    cmp al, 0x2E       
    je ctrl_c_detected
    cmp al, 0x20        
    je ctrl_d_detected

no_control_key:
    call keyboard_handler
    jmp exit

control_pressed:
    mov byte [ctrl_pressed], 1  ; Marcar que Ctrl está presionado
    jmp exit

control_released:
    mov byte [ctrl_pressed], 0  ; Marcar que Ctrl fue soltado
    ; saving an array of registers: RAX, RBX, RCX, RDX, RSI, RDI, RBP, R8, R9, R10, R11, R12, R13
    ; R14, R15, RSP, RIP, RFLAGS
    mov [registers+8*1], rbx
    mov [registers+8*2], rcx
    mov [registers+8*3], rdx
    mov [registers+8*4], rsi
    mov [registers+8*5], rdi
    mov [registers+8*6], rbp
    mov [registers+8*7], r8
    mov [registers+8*8], r9
    mov [registers+8*9], r10
    mov [registers+8*10], r11
    mov [registers+8*11], r12
    mov [registers+8*12], r13
    mov [registers+8*13], r14
    mov [registers+8*14], r15

    mov rax, rsp
    add rax, 160              ; volvemos a antes de pushear los registros
    mov [registers + 8*15], rax  ; RSP

    mov rax, [rsp+15*8]
    mov [registers + 8*16], rax ; RIP

    mov rax, [rsp + 14*8]     ; RAX
    mov [registers], rax

    mov rax, [rsp+15*9]
    mov [registers + 8*17], rax ; RFLAGS

    mov byte [capturedReg], 1
    jmp exit

ctrl_c_detected:
    call killForegroundProcess   ; Llamar a killForegroundProcess
    jmp exit

ctrl_d_detected:
	call setEofFlag
    jmp exit

exit:
    mov al, 20h
    out 20h, al
    popState
    iretq

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5

;Syscall
_syscallHandler:
	pushState
	mov rbp, rsp

	push r9
	mov r9, r8
	mov r8, r10
	mov rcx, rdx
	mov rdx, rsi
	mov rsi, rdi
	mov rdi, rax
	call syscallDispatcher ;reordenamos los registros
	mov [aux], rax

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	pop r9
	mov rsp, rbp
	popState
	mov rax, [aux]
	iretq


;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;OP code Exception
_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret



SECTION .bss
	aux resq 1
	exceptregs resq 18	;registros para la excepcion
	registers resq 18		;registros para el teclado
	capturedReg resb 1		;flag para saber si se capturo un teclado
	ctrl_pressed resb 1		;flag para saber si se presiono ctrl
GLOBAL cpuVendor
GLOBAL get_key

GLOBAL get_sec
GLOBAL get_min
GLOBAL get_hour
GLOBAL beep
GLOBAL stop_beep

GLOBAL callTimerTick

;vectores
GLOBAL htl_lib

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret


get_key:
	push rbp
    mov rbp, rsp

    xor rax, rax
    in al, 60h

    mov rsp, rbp
    pop rbp
    ret

get_sec:
	push rbp
	mov  rbp,rsp
	mov al, 0
	out 70h, al
	in al, 71h
	mov rsp,rbp
	pop rbp
	ret

get_min:
	push rbp
	mov  rbp,rsp
	mov al, 2
	out 70h, al
	in al, 71h
	mov rsp,rbp
	pop rbp
	ret


get_hour:
	push rbp
	mov  rbp,rsp
	mov rax,0
	mov al, 4
	out 70h, al
	in al, 71h
	mov rsp,rbp
	pop rbp
	ret

beep:
	push rbp
    push rdx
	mov rbp, rsp

	mov al, 0xB6
	out 43h, al

    mov rdx, 0
    mov rax, 1193180
    div rdi
    
	out 42h, al
	mov al, ah
	out 42h, al

	in al, 61h
 	or al, 03h
	out 61h, al

	mov rsp, rbp
    pop rdx
	pop rbp
	ret

stop_beep:
  	in al, 61h
	and al, 0xFC
	out 61h, al
  	ret


htl_lib:
	sti
	hlt
	ret

callTimerTick:
	int 20h
	ret
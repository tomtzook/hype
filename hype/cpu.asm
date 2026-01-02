bits 64
section .text

global asm_cpu_store_registers
global asm_cpu_load_registers

asm_cpu_store_registers:
    ; rcx = address to struct cpu_registers_t
    mov [rcx+00h], rax
    mov [rcx+08h], rbx
    ; rcx is used for the pointer so we cannot save it here
    mov [rcx+18h], rdx
    mov [rcx+20h], r8
    mov [rcx+28h], r9
    mov [rcx+30h], r10
    mov [rcx+38h], r11
    mov [rcx+40h], r12
    mov [rcx+48h], r13
    mov [rcx+50h], r14
    mov [rcx+58h], r15
    mov [rcx+60h], rsi
    mov [rcx+68h], rdi
    mov [rcx+78h], rbp

    ; original rsp
    lea rax, [rsp+8h]
    mov [rcx+80h], rax

    ; rip
    mov rax, [rsp]
    mov [rcx+88h], rax

    ; rflags
    pushfq
    pop rax
    mov [rcx+70h], rax

    ; 4. Save Segment Registers
    mov [rcx+90h], cs
    mov [rcx+92h], ds
    mov [rcx+94h], es
    mov [rcx+96h], fs
    mov [rcx+98h], gs
    mov [rcx+9ah], ss

    ret


asm_cpu_load_registers:
    ; rcx = address to struct cpu_registers_t
    ; put ss, rsp, rflags, cs, rip on stack, will be popped in the end by iretq
    movzx rax, word [rcx+9ah] ; ss
    push rax
    mov rax, [rcx+80h]        ; rsp
    push rax
    mov rax, [rcx+70h]        ; rflags
    push rax
    movzx rax, word [rcx+90h] ; cs
    push rax
    mov rax, [rcx+88h]        ; rip
    push rax

    ; restore other registers
    mov rbx, [rcx+08h]
    mov rdx, [rcx+18h]
    mov r8,  [rcx+20h]
    mov r9,  [rcx+28h]
    mov r10, [rcx+30h]
    mov r11, [rcx+38h]
    mov r12, [rcx+40h]
    mov r13, [rcx+48h]
    mov r14, [rcx+50h]
    mov r15, [rcx+58h]
    mov rsi, [rcx+60h]
    mov rdi, [rcx+68h]
    mov rbp, [rcx+78h]

    ; restore rax and rcx, must be last
    mov rax, rcx
    mov rcx, [rax+10h]
    mov rax, [rax+00h]

    iretq

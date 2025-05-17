bits 64
section .text

global asm_cpu_store_registers
global asm_cpu_load_registers

asm_cpu_store_registers:
    ; rcx = address to struct cpu_registers_t
    mov [rcx+0h], rax
    mov [rcx+8h], rbx
    mov [rcx+10h], rcx  ; this place is used by other code, don't change it
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
    mov word [rcx+90h], cs
    mov word [rcx+92h], ds
    mov word [rcx+94h], es
    mov word [rcx+96h], fs
    mov word [rcx+98h], gs
    mov word [rcx+9ah], ss

    lea rax, [rsp+8h]  ; get rsp
    mov [rcx+80h], rax
    lea rax, [rsp]  ; get return address as rip
    mov [rcx+88h], rax

    ; rflags
    pushfq
    pop rax
    mov [rcx+70h], rax

    ret


asm_cpu_load_registers:
    ; rcx = address to struct cpu_registers_t
    mov rax, [rcx+0h]
    mov rbx, [rcx+8h]
    mov rdx, [rcx+18h]
    mov r8, [rcx+20h]
    mov r9, [rcx+28h]
    mov r10, [rcx+30h]
    mov r11, [rcx+38h]
    mov r12, [rcx+40h]
    mov r13, [rcx+48h]
    mov r14, [rcx+50h]
    mov r15, [rcx+58h]
    mov rsi, [rcx+60h]
    mov rdi, [rcx+68h]
    mov rbp, [rcx+78h]

    ; intentionally not touching segment selectors

    ; rflags
    mov rax, [rcx+70h]
    push rax
    popfq

    ; we now have a new stack
    mov rsp, [rcx+80h]

    ; write rip into return address
    mov rax, [rcx+88h]
    mov [rsp], rax

    ; rcx must be last
    mov rax, rcx
    mov rcx, [rax+10h]

    ret

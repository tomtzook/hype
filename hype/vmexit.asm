bits 64
section .text

extern asm_cpu_store_registers
extern vm_exit_handler

global asm_vm_exit


asm_vm_exit:
    push rcx            ; because we are overriding rcx to load the struct, save it here to be returned to stack later
    lea rcx, [rsp+8h]   ; context registers are at the top of the stack

    call asm_cpu_store_registers

    ; return real rcx and rsp
    pop rax
    mov [rcx+10h], rax
    mov [rcx+80h], rsp

    sub rsp, 20h
    jmp vm_exit_handler

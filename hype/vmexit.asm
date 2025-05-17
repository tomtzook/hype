bits 64
section .text

extern asm_cpu_store_registers
extern vm_exit_handler

global asm_vm_exit


asm_vm_exit:
    push rcx            ; because we are overriding rcx to load the struct, save it here to be returned to stack later
    lea rcx, [rsp+18h]  ; context registers are at the top of the stack

    call asm_cpu_store_registers

    ; return real rcx
    pop rax
    mov [rcx+10h], rax

    sub rsp, 28h
    jmp vm_exit_handler
